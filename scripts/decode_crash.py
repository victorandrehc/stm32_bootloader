#!/usr/bin/env python3
"""
Decode a HardFault crash dump captured by hardfault_c() into a human-readable
backtrace.

Reads the textual crash dump produced by crash_dump_print() (from --dump or
stdin), then uses addr2line and objdump to:

  - Decode CFSR/HFSR fault flags into named bits.
  - Resolve PC and LR to function name + source:line (C++ demangled).
  - Print r0-r3 from the hardware-stacked frame, with hints for flash/RAM
    pointers (caveat: these are register values at fault, not necessarily
    the original function arguments).
  - Walk the captured stack snapshot for return-address candidates
    (0x0800xxxx) and validate each by checking that the instruction at
    `addr - 4` in the ELF is `bl`/`blx`. Marks each candidate as either
    confirmed (preceded by a real call) or unconfirmed (coincidental).
  - For each frame whose function is in the ELF DWARF info, also print
    its parameter list (names and types). For the deepest (faulting)
    frame, the first 4 parameters are mapped to r0-r3 from the captured
    register state. For ancestor frames, parameter values are not shown
    (recovery would require full DWARF CFI unwinding); only names/types
    are displayed.

Requires pyelftools for the DWARF parsing (`pip install pyelftools`).
Without pyelftools the script still runs but skips the parameter section.
"""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple


# ---- bit decoders ---------------------------------------------------------

UFSR_BITS = {
    0: "UNDEFINSTR",
    1: "INVSTATE",
    2: "INVPC",
    3: "NOCP",
    8: "UNALIGNED",
    9: "DIVBYZERO",
}
BFSR_BITS = {
    0: "IBUSERR",
    1: "PRECISERR",
    2: "IMPRECISERR",
    3: "UNSTKERR",
    4: "STKERR",
    5: "LSPERR",
    7: "BFARVALID",
}
MMFSR_BITS = {
    0: "IACCVIOL",
    1: "DACCVIOL",
    3: "MUNSTKERR",
    4: "MSTKERR",
    5: "MLSPERR",
    7: "MMARVALID",
}
HFSR_BITS = {
    1: "VECTTBL",
    30: "FORCED",
    31: "DEBUGEVT",
}


def decode_bits(value: int, mapping: Dict[int, str]) -> List[str]:
    return [name for bit, name in sorted(mapping.items()) if value & (1 << bit)]


def decode_fault(cfsr: int, hfsr: int) -> str:
    parts = []
    mmfsr = cfsr & 0xFF
    bfsr = (cfsr >> 8) & 0xFF
    ufsr = (cfsr >> 16) & 0xFFFF
    if mmfsr:
        parts.append("MemManage." + ",".join(decode_bits(mmfsr, MMFSR_BITS)))
    if bfsr:
        parts.append("BusFault." + ",".join(decode_bits(bfsr, BFSR_BITS)))
    if ufsr:
        parts.append("UsageFault." + ",".join(decode_bits(ufsr, UFSR_BITS)))
    if hfsr:
        parts.append("HardFault." + ",".join(decode_bits(hfsr, HFSR_BITS)))
    return " | ".join(parts) if parts else "(no flags set)"


# ---- address classification ----------------------------------------------

def is_flash_address(value: int) -> bool:
    """STM32F4 internal flash range: 0x08000000 - 0x080FFFFF."""
    return 0x08000000 <= value < 0x08100000


def is_ram_address(value: int) -> bool:
    """STM32F4 SRAM range: 0x20000000 - 0x2001FFFF (96 KB)."""
    return 0x20000000 <= value < 0x20020000


# ---- dump parser ----------------------------------------------------------

DUMP_KEYS = (
    "sp_at_fault",
    "cfsr", "hfsr", "mmfar", "bfar",
    "r0", "r1", "r2", "r3", "r12", "lr", "pc", "xpsr",
)


def parse_dump(text: str) -> dict:
    """Pull named hex fields and stack lines out of the dump text."""
    result: dict = {}
    for k in DUMP_KEYS:
        m = re.search(rf"\b{k}\s*[:=]\s*0?x?([0-9a-fA-F]+)", text, re.IGNORECASE)
        if m:
            result[k] = int(m.group(1), 16)

    stack: List[Tuple[int, int]] = []
    for m in re.finditer(r"0x([0-9a-fA-F]{6,8})\s*:\s*0x([0-9a-fA-F]{1,8})", text):
        addr = int(m.group(1), 16)
        val = int(m.group(2), 16)
        if not is_ram_address(addr):
            continue
        stack.append((addr, val))
    result["stack"] = stack
    return result


# ---- toolchain wrappers ---------------------------------------------------

class Toolchain:
    def __init__(self, addr2line: Path, objdump: Path, elf: Path) -> None:
        self.addr2line = addr2line
        self.objdump = objdump
        self.elf = elf
        self._a2l_cache: Dict[int, Tuple[str, str]] = {}
        self._disasm: Optional[Dict[int, Tuple[str, str]]] = None

    def addr_to_line(self, addr: int) -> Tuple[str, str]:
        addr &= ~1  # strip Thumb bit
        if addr in self._a2l_cache:
            return self._a2l_cache[addr]
        try:
            out = subprocess.check_output(
                [str(self.addr2line), "-fCe", str(self.elf), f"0x{addr:08x}"],
                text=True,
                stderr=subprocess.DEVNULL,
            ).strip().splitlines()
        except subprocess.CalledProcessError:
            self._a2l_cache[addr] = ("?", "?")
            return self._a2l_cache[addr]
        func = out[0] if out else "?"
        src = out[1] if len(out) > 1 else "?"
        self._a2l_cache[addr] = (func, src)
        return self._a2l_cache[addr]

    def disasm(self) -> Dict[int, Tuple[str, str]]:
        """Return {address: (mnemonic, raw_line)} for the whole ELF."""
        if self._disasm is not None:
            return self._disasm
        try:
            raw = subprocess.check_output(
                [str(self.objdump), "-d", str(self.elf)],
                text=True,
                stderr=subprocess.DEVNULL,
            )
        except subprocess.CalledProcessError as e:
            sys.exit(f"objdump failed: {e}")
        instr_re = re.compile(
            r"^\s*([0-9a-fA-F]+):\s+[0-9a-fA-F ]+\s+([a-zA-Z][\w.]*)"
        )
        out: Dict[int, Tuple[str, str]] = {}
        for line in raw.splitlines():
            m = instr_re.match(line)
            if m:
                out[int(m.group(1), 16)] = (m.group(2).lower(), line.rstrip())
        self._disasm = out
        return out

    def is_real_return_addr(self, ret_addr: int) -> Tuple[bool, str]:
        """True if the instruction at ret_addr - 4 is `bl` or `blx`."""
        ret_addr &= ~1
        d = self.disasm()
        candidate = ret_addr - 4
        if candidate in d:
            mnem, line = d[candidate]
            if mnem in ("bl", "blx") or mnem.startswith("bl."):
                return (True, f"preceded by `{mnem}` at 0x{candidate:08x}")
        return (False, f"no `bl`/`blx` at 0x{candidate:08x}")


# ---- DWARF helper for parameter extraction --------------------------------

class DwarfHelper:
    """Minimal DWARF reader that maps an address to a list of (param_name,
    param_type) tuples. Returns None if pyelftools isn't available or the
    ELF lacks DWARF info."""

    def __init__(self, elf_path: Path) -> None:
        self.available = False
        self._subprograms: List[Tuple[int, int, object, object]] = []
        try:
            from elftools.elf.elffile import ELFFile  # type: ignore
        except ImportError:
            return

        self._fh = open(str(elf_path), "rb")
        elf = ELFFile(self._fh)
        if not elf.has_dwarf_info():
            return

        self._dwarf = elf.get_dwarf_info()
        for cu in self._dwarf.iter_CUs():
            for die in cu.iter_DIEs():
                if die.tag != "DW_TAG_subprogram":
                    continue
                if "DW_AT_low_pc" not in die.attributes:
                    continue
                low = die.attributes["DW_AT_low_pc"].value
                hi_attr = die.attributes.get("DW_AT_high_pc")
                if hi_attr is None:
                    continue
                if hi_attr.form == "DW_FORM_addr":
                    high = hi_attr.value
                else:
                    high = low + hi_attr.value
                self._subprograms.append((low, high, die, cu))
        self._subprograms.sort(key=lambda x: x[0])
        self.available = True

    def signature(self, addr: int) -> Optional[List[Tuple[str, str]]]:
        """Return [(param_name, param_type_str), ...] for the function
        containing `addr`, or None if not found."""
        if not self.available:
            return None
        addr &= ~1
        for low, high, die, cu in self._subprograms:
            if low <= addr < high:
                return self._params_of(die, cu)
        return None

    def _params_of(self, sub_die, cu) -> List[Tuple[str, str]]:
        params: List[Tuple[str, str]] = []
        for child in sub_die.iter_children():
            if child.tag != "DW_TAG_formal_parameter":
                continue
            name = self._attr_str(child, "DW_AT_name", "?")
            type_die = self._resolve_type(child.attributes.get("DW_AT_type"), cu)
            params.append((name, self._type_str(type_die)))
        return params

    def _resolve_type(self, type_attr, owner_cu):
        if type_attr is None:
            return None
        try:
            if type_attr.form == "DW_FORM_ref_addr":
                return self._dwarf.get_DIE_from_refaddr(type_attr.value)
            return self._dwarf.get_DIE_from_refaddr(
                owner_cu.cu_offset + type_attr.value
            )
        except Exception:
            return None

    def _type_str(self, die, depth: int = 0) -> str:
        if die is None:
            return "void"
        if depth > 8:
            return "..."
        tag = die.tag
        inner_attr = die.attributes.get("DW_AT_type")
        cu = die.cu
        if tag == "DW_TAG_base_type":
            return self._attr_str(die, "DW_AT_name", "?")
        if tag == "DW_TAG_pointer_type":
            inner = self._type_str(self._resolve_type(inner_attr, cu), depth + 1)
            return inner + " *"
        if tag == "DW_TAG_reference_type":
            inner = self._type_str(self._resolve_type(inner_attr, cu), depth + 1)
            return inner + " &"
        if tag == "DW_TAG_const_type":
            inner = self._type_str(self._resolve_type(inner_attr, cu), depth + 1)
            return "const " + inner
        if tag == "DW_TAG_volatile_type":
            inner = self._type_str(self._resolve_type(inner_attr, cu), depth + 1)
            return "volatile " + inner
        if tag == "DW_TAG_typedef":
            return self._attr_str(die, "DW_AT_name", "?")
        if tag == "DW_TAG_structure_type":
            return "struct " + self._attr_str(die, "DW_AT_name", "<anon>")
        if tag == "DW_TAG_union_type":
            return "union " + self._attr_str(die, "DW_AT_name", "<anon>")
        if tag == "DW_TAG_enumeration_type":
            return "enum " + self._attr_str(die, "DW_AT_name", "<anon>")
        if tag == "DW_TAG_array_type":
            inner = self._type_str(self._resolve_type(inner_attr, cu), depth + 1)
            return inner + "[]"
        if tag == "DW_TAG_subroutine_type":
            ret = self._type_str(self._resolve_type(inner_attr, cu), depth + 1)
            return f"{ret}(*)(...)"
        return tag

    @staticmethod
    def _attr_str(die, attr: str, default: str = "?") -> str:
        if attr in die.attributes:
            v = die.attributes[attr].value
            if isinstance(v, bytes):
                return v.decode("utf-8", errors="replace")
            return str(v)
        return default


# ---- argument hint -------------------------------------------------------

def hint_for(value: int, tc: Toolchain) -> str:
    if value == 0:
        return ""
    if is_flash_address(value):
        func, src = tc.addr_to_line(value)
        return f"  -> code/rodata: {func} ({src})"
    if is_ram_address(value):
        return "  -> RAM pointer"
    return ""


def format_signature(params: List[Tuple[str, str]]) -> str:
    if not params:
        return "(void)"
    return "(" + ", ".join(f"{t} {n}" for n, t in params) + ")"


# ---- arg validation ------------------------------------------------------

def validated_path(p: str, what: str) -> Path:
    path = Path(p).expanduser()
    if not path.exists():
        sys.exit(f"error: --{what} path does not exist: {path}")
    if not path.is_file():
        sys.exit(f"error: --{what} is not a regular file: {path}")
    if what != "elf" and not os.access(path, os.X_OK):
        sys.exit(f"error: --{what} is not executable: {path}")
    return path


# ---- main ----------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    ap.add_argument("--elf", required=True,
                    help="firmware ELF that produced the dump")
    ap.add_argument("--addr2line", required=True,
                    help="path to arm-none-eabi-addr2line")
    ap.add_argument("--objdump", required=True,
                    help="path to arm-none-eabi-objdump")
    ap.add_argument("--dump", default=None,
                    help="path to crash dump text (default: stdin)")
    args = ap.parse_args()

    elf = validated_path(args.elf, "elf")
    a2l = validated_path(args.addr2line, "addr2line")
    objd = validated_path(args.objdump, "objdump")

    text = Path(args.dump).read_text() if args.dump else sys.stdin.read()
    info = parse_dump(text)

    if "pc" not in info and "lr" not in info and not info["stack"]:
        sys.stderr.write("error: no recognizable fields found in dump\n")
        return 2

    tc = Toolchain(a2l, objd, elf)
    dw = DwarfHelper(elf)
    if not dw.available:
        sys.stderr.write(
            "warning: pyelftools missing or ELF has no DWARF — "
            "parameter signatures will not be shown\n"
        )

    # ---- fault decode ----
    cfsr = info.get("cfsr", 0)
    hfsr = info.get("hfsr", 0)
    print("=== Fault ===")
    print(f"  CFSR = 0x{cfsr:08x}    HFSR = 0x{hfsr:08x}")
    print(f"  decoded: {decode_fault(cfsr, hfsr)}")
    if (cfsr & (1 << 7)) and "mmfar" in info:
        print(f"  MMFAR = 0x{info['mmfar']:08x}")
    if (cfsr & (1 << 15)) and "bfar" in info:
        print(f"  BFAR  = 0x{info['bfar']:08x}")
    if "sp_at_fault" in info:
        print(f"  SP_at_fault: 0x{info['sp_at_fault']:08x}")
    print()

    # ---- innermost frame ----
    print("=== Innermost frame ===")
    pc = info.get("pc")
    lr = info.get("lr")
    if pc is not None:
        func, src = tc.addr_to_line(pc)
        params = dw.signature(pc)
        sig = format_signature(params) if params is not None else ""
        print(f"  PC = 0x{pc:08x}  {func}{sig}")
        print(f"            at {src}")
        # Map r0-r3 to first 4 params of PC's function (best-effort)
        if params is not None and params:
            print(f"  parameters (r0-r3 = arg snapshot at fault):")
            for i, (pname, ptype) in enumerate(params[:4]):
                key = f"r{i}"
                v = info.get(key, 0)
                hint = hint_for(v, tc) if v else ""
                print(f"    {ptype} {pname} = 0x{v:08x}  (in {key}){hint}")
            if len(params) > 4:
                print(f"    {len(params) - 4} more param(s) on the stack "
                      f"(not recovered)")
        else:
            print(f"  r0-r3 (arg snapshot at fault — value-only, no signature):")
            for i in range(4):
                v = info.get(f"r{i}", 0)
                print(f"    r{i} = 0x{v:08x}{hint_for(v, tc)}")
    if lr is not None:
        func, src = tc.addr_to_line(lr)
        params = dw.signature(lr)
        sig = format_signature(params) if params is not None else ""
        valid, reason = tc.is_real_return_addr(lr)
        mark = "[ok]" if valid else "[??]"
        print(f"  LR = 0x{lr:08x}  {mark}  {func}{sig}")
        print(f"            at {src}")
        if not valid:
            print(f"            note: {reason}")
    print()

    # ---- ancestor frames ----
    print("=== Ancestor frames (heuristic, oldest-first as found in stack) ===")
    candidates = [(a, v) for a, v in info["stack"] if is_flash_address(v)]
    if not candidates:
        print("  (no flash-shaped candidates in stack snapshot)")
    else:
        for stack_addr, ret_addr in candidates:
            valid, reason = tc.is_real_return_addr(ret_addr)
            func, src = tc.addr_to_line(ret_addr)
            params = dw.signature(ret_addr)
            sig = format_signature(params) if params is not None else ""
            mark = "[ok]" if valid else "[??]"
            print(f"  [0x{stack_addr:08x}] -> 0x{ret_addr:08x}  {mark}  "
                  f"{func}{sig}")
            print(f"               at {src}")
            if not valid:
                print(f"               note: {reason}")
            if params:
                print(f"               parameters (values not recoverable "
                      f"without unwinding):")
                for pname, ptype in params:
                    print(f"                 {ptype} {pname}")
        print()
        print("  [ok] = preceded by a `bl`/`blx` (real return address)")
        print("  [??] = looks like flash, but no `bl` immediately before it")
        print("  Param values shown only for the innermost frame "
              "(from r0-r3 at fault).")

    return 0


if __name__ == "__main__":
    sys.exit(main())
