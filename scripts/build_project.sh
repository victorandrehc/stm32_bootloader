#!/bin/bash

echo "Configuring Project"
cmake -B build -G "Ninja" -DARM_TOOLCHAIN_DIR="/opt/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin" -DCMAKE_BUILD_TYPE=Debug

echo "Building Project"
cmake --build build