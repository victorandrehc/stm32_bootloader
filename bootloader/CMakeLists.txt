set(SOURCE_FILES
        ../Inc/main.h
        ../Inc/stm32f4xx_hal_conf.h
        ../Inc/stm32f4xx_it.h
        Src/main.c
        Src/stm32f4xx_hal_msp.c
        Src/stm32f4xx_it.c
        Src/system_stm32f4xx.c
        Src/startup_stm32f401xe.s
        Src/syscalls.c
        ../boot_control/Src/boot_config.c
        Src/uart_handler.h
        Src/uart_handler.c
        Src/flash_handler.h
        Src/flash_handler.c
        )

set(EXECUTABLE ${PROJECT_NAME}_bootloader.out)

add_executable(${EXECUTABLE} ${STM32CUBEMX_GENERATED_FILES} ${SOURCE_FILES})
target_link_libraries(${EXECUTABLE} drivers serial_protocol)

target_compile_definitions(${EXECUTABLE} PRIVATE
        -DUSE_HAL_DRIVER
        -DSTM32F401xE
        )

target_include_directories(${EXECUTABLE} PRIVATE
        ../Inc
        ../Drivers/STM32F4xx_HAL_Driver/Inc
        ../Drivers/CMSIS/Device/ST/STM32F4xx/Include
        ../Drivers/CMSIS/Include
        ../boot_control/Inc
        ../serial_protocol/mcu/Inc
        )

target_compile_options(${EXECUTABLE} PRIVATE
        ${CPU_PARAMETERS}
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -ffunction-sections
        -fdata-sections
        $<$<COMPILE_LANGUAGE:CXX>:
        -Wno-volatile
        #        -Wold-style-cast
        #        -Wuseless-cast
        #        -Wsuggest-override
        -fno-rtti
        -fno-exceptions
        -fno-builtin
        -fno-math-errno
        -fno-use-cxa-atexit
        -flto>

        $<$<CONFIG:Debug>:-Og -g3 -ggdb>
        $<$<CONFIG:Release>:-Os>)


set(LINKING_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/stm32f401.ld)

target_link_options(${EXECUTABLE} PRIVATE
        -T${LINKING_SCRIPT}
        ${CPU_PARAMETERS}
        -Wl,-Map=${EXECUTABLE}.map
        -specs=nosys.specs
        -specs=nano.specs
        -Wl,--start-group
        -Wl,--gc-sections
        -lc
        -lm
        -lstdc++
        -Wl,--end-group
        -Wl,--print-memory-usage
        -flto
        -Wl,--no-warn-rwx-segment
        #THis here tells the export the bootloaderSymbol to be used on the linker script
        -Wl,--defsym,bootloaderSymbol=1)

# Improve clean target
set_target_properties(${EXECUTABLE} PROPERTIES ADDITIONAL_CLEAN_FILES
        "${PROJECT_NAME}.bin;${PROJECT_NAME}.hex;${PROJECT_NAME}.map")

# Print executable size
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE})

# Create hex file
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${PROJECT_NAME}_bootloader.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${PROJECT_NAME}_bootloader.bin)