set(DRIVER_LIB drivers)

set(STM32CUBEMX_GENERATED_FILES
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
        STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_crc.c
        )
add_library(${DRIVER_LIB}
        ${STM32CUBEMX_GENERATED_FILES})

target_include_directories(${DRIVER_LIB} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../Inc
        STM32F4xx_HAL_Driver/Inc
        CMSIS/Device/ST/STM32F4xx/Include
        CMSIS/Include
        )

target_compile_definitions(${DRIVER_LIB} PRIVATE
        -DUSE_HAL_DRIVER
        -DSTM32F401xE
        )

target_compile_options(${DRIVER_LIB} PRIVATE
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

target_link_options(${DRIVER_LIB} PRIVATE
        -T${LINKING_SCRIPT}
        ${CPU_PARAMETERS}
        -Wl,-Map=${DRIVER_LIB}.map
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

