

# CMake-STM32
This is a basic project using STM32Hal for the STM32F401RE using cmake and gcc-arm and serve as an example and template
for someone wanting to apply ST's Hal library on a more professional environment than Cube IDE or their many look a like.
This project is scoped to work on STM32F401RE only but can easily be modified for other targets. For this a change on
the Drivers folder might be necessary since the target might use a different HAL.


## Dependencies
- CMake>=3.25
- Ninja
- gcc-arm

## How to build
1. Configure the project via
`cmake -B build -G "Ninja" -DARM_TOOLCHAIN_DIR="/opt/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin" -DCMAKE_BUILD_TYPE=Debug`
2. Build the project via
`cmake --build build` 
3. Flash the project via
`openocd -f board/st_nucleo_f4.cfg -c 'program build/cmake_stm32.bin 0x08000000' -c 'verify_image_checksum build/cmake_stm32.bin' -c 'sleep 200' -c 'reset run' -c 'exit'`
4. Open gdb via
`openocd -f interface/stlink.cfg -f target/stm32f4x.cfg`
`gdb -ex "target remote localhost:3333" -ex "monitor reset halt" build/cmake_stm32.out`

## Toolchain File
The toolchain file on this project is located on `arm-none-eabi-gcc.cmake` and presumes that the gcc arm is located on the
`"/opt/arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-eabi/bin"` folder, if your toolchain is located on a different folder,
a modification on the toolchain file is needed.

