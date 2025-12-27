set(SERIAL_PROTOCOL serial_protocol)

set(SERIAL_PROTOCOL_SOURCE
        Src/serial_protocol.c
        Src/serial_process_frame.h
        Src/serial_process_frame.c
        Src/serial_api.h
        Src/serial_api.c
)

add_library(${SERIAL_PROTOCOL}
        ${SERIAL_PROTOCOL_SOURCE})

target_include_directories(${SERIAL_PROTOCOL} PRIVATE
        Inc
        )

target_compile_definitions(${SERIAL_PROTOCOL} PRIVATE
        -DUSE_HAL_DRIVER
        -DSTM32F401xE
        )

target_link_libraries(${SERIAL_PROTOCOL} drivers)

target_compile_options(${SERIAL_PROTOCOL} PRIVATE
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

target_link_options(${SERIAL_PROTOCOL} PRIVATE
        -T${LINKING_SCRIPT}
        ${CPU_PARAMETERS}
        -Wl,-Map=${SERIAL_PROTOCOL}.map
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

