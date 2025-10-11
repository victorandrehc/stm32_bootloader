set(BOOT_CONTROL boot_control)

set(BOOT_CONTROL_SOURCE
        Src/boot_config.c

)

add_library(${BOOT_CONTROL}
        ${BOOT_CONTROL_SOURCE})

target_include_directories(${BOOT_CONTROL} PRIVATE
        Inc
        ../Drivers/STM32F4xx_HAL_Driver/Inc
        ../Drivers/CMSIS/Device/ST/STM32F4xx/Include
        ../Drivers/CMSIS/Include
         ../Inc
        )

target_compile_definitions(${BOOT_CONTROL} PRIVATE
        -DUSE_HAL_DRIVER
        -DSTM32F401xE
        )

target_link_libraries(${BOOT_CONTROL} drivers)

target_compile_options(${BOOT_CONTROL} PRIVATE
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

target_link_options(${BOOT_CONTROL} PRIVATE
        -T${LINKING_SCRIPT}
        ${CPU_PARAMETERS}
        -Wl,-Map=${BOOT_CONTROL}.map
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
        -Wl,--no-warn-rwx-segment)

