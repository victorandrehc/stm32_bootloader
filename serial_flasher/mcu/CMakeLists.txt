set(SERIAL_FLASHER serial_flasher)

set(SERIAL_FLASHER_SOURCE
        Src/serial_flasher.c
        Src/serial_process_frame.h
        Src/serial_process_frame.c
        Src/serial_api.h
        Src/serial_api.c
)

add_library(${SERIAL_FLASHER}
        ${SERIAL_FLASHER_SOURCE})

target_include_directories(${SERIAL_FLASHER} PRIVATE
        Inc
        )

target_compile_definitions(${SERIAL_FLASHER} PRIVATE
        -DUSE_HAL_DRIVER
        -DSTM32F401xE
        )

target_link_libraries(${SERIAL_FLASHER} drivers)

target_compile_options(${SERIAL_FLASHER} PRIVATE
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

target_link_options(${SERIAL_FLASHER} PRIVATE
        -T${LINKING_SCRIPT}
        ${CPU_PARAMETERS}
        -Wl,-Map=${SERIAL_FLASHER}.map
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

