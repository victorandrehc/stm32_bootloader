#!/bin/bash

openocd -f flash_openocd.cfg
# echo "FLASHING APP"
# openocd -f board/st_nucleo_f4.cfg -c 'program build/app/cmake_stm32_app.bin 0x08000000' -c 'verify_image_checksum build/app/cmake_stm32_app.bin' -c 'sleep 200' -c 'reset run' -c 'exit'