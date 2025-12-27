#!/bin/bash
# openocd -f interface/stlink.cfg -f target/stm32f4x.cfg &

sleep 1

ELF_FILE=${1:-build/bootloader/cmake_stm32_bootloader.out}

gdb \
  -ex "target remote localhost:3333" \
  -ex "monitor reset halt" \
  --tui \
  "$ELF_FILE"
