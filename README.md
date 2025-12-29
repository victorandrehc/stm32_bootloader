# CMake-STM32

> **Disclaimer**  
> This project is intended **for academic and educational purposes only**. It is **not production-ready** and must not be used in safety-critical or commercial products. The project currently lacks essential production features such as **firmware signing, secure boot, rollback protection, and cryptographic validation**.

This repository provides a minimal STM32 project based on **STM32 HAL**, built using **CMake** and **gcc-arm-none-eabi**. It targets the **STM32F401RE** microcontroller and is intended to serve as both an example and a reusable template for developers who want to use ST’s HAL library in a more professional and flexible environment than STM32CubeIDE or similar IDE-based workflows.

The project is currently scoped to the **STM32F401RE** only, but it can be adapted to other STM32 targets with minimal effort. When switching targets, updates to the `Drivers` directory may be required, as different devices rely on different HAL implementations.

---

## Dependencies

The following tools are required to build, flash, and debug the project:

- CMake ≥ 3.25  
- Ninja  
- gcc-arm-none-eabi  
- Python 3  
- pip  
- picocom  

---

## How to Build

1. Configure and build the project using:
   ```bash
   ./scripts/build_project.sh
   ```

2. Flash the bootloader:
   ```bash
   ./scripts/flash.sh
   ```

3. (Optional) Debug using GDB:

   - Start the OpenOCD server:
     ```bash
     openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
     ```

   - Start the GDB client:
     ```bash
     gdb -ex "target remote localhost:3333" -ex "monitor reset halt" build/bootloader/cmake_stm32_bootloader.out
     ```
     or
     ```bash
     gdb -ex "target remote localhost:3333" -ex "monitor reset halt" build/app/cmake_stm32_app.out
     ```

   - Alternatively, you can launch GDB using the helper script:
     ```bash
     ./scripts/gdb.sh
     ```

4. (Optional – Highly Recommended) Open a serial terminal:
   ```bash
   picocom --imap lfcrlf -b 115200 /dev/ttyACM0
   ```

---

## Flashing the Application via Python Flasher

### 1. Enter DFU Mode

If the bootloader or demo application is currently running, press the **B1** button to reset the MCU into **DFU mode**. When DFU mode is active, you should see output similar to the following:

```bash
BOOTLOADER:    ____              __
BOOTLOADER:   / __ )____  ____  / /_
BOOTLOADER:  / __  / __ \/ __ \/ __/
BOOTLOADER: / /_/ / /_/ / /_/ / /_
BOOTLOADER: /_____/\____/\____/\__/
BOOTLOADER:
BOOTLOADER: Press Button to Enter DFU mode
BOOTLOADER: MAGIC NUMBER: 0xdeadbeef RESET_REASON: FIRMWARE UPDATE
BOOTLOADER: Entering in DFU ..
```

The bootloader will remain in DFU mode until it is either reset or a new firmware is flashed. If no valid application is found, the bootloader will automatically enter DFU mode.

---

### 2. Flash the Demo Application

1. Connect an **FTDI** adapter to **USART1**:
   - **PA9** → USART1_TX  
   - **PA10** → USART1_RX  

2. Run the Python flasher:
   ```bash
   python serial_protocol/python/serial_protocol.py \
     build/app/cmake_stm32_app.bin \
     --tty_port /dev/ttyUSB0 \
     --baudrate 115200
   ```

---

## Notes

- This project is designed to be IDE-agnostic and script-driven.
- It is well suited for experimentation, learning, and academic exploration.
- Contributions and improvements are welcome.
