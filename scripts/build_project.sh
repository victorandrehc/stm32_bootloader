#!/bin/bash


DEFAULT_TOOLCHAIN_DIR="/opt/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin"
DEFAULT_BUILD_TYPE="Debug"

ARM_TOOLCHAIN_DIR="$DEFAULT_TOOLCHAIN_DIR"
BUILD_TYPE="$DEFAULT_BUILD_TYPE"

VALID_BUILD_TYPES=("Debug" "Release" "RelWithDebInfo" "MinSizeRel")

usage() {
    echo "Usage: $0 [options]"
    echo
    echo "Options:"
    echo "  -t, --toolchain DIR     ARM toolchain directory"
    echo "  -b, --build-type TYPE   Build type (Debug, Release, RelWithDebInfo, MinSizeRel)"
    echo "  -h, --help              Show this help message"
}

# Parse options
PARSED_OPTS=$(getopt -o t:b:h --long toolchain:,build-type:,help -- "$@")
if [[ $? -ne 0 ]]; then
    usage
    exit 1
fi

eval set -- "$PARSED_OPTS"

while true; do
    case "$1" in
        -t|--toolchain)
            ARM_TOOLCHAIN_DIR="$2"
            shift 2
            ;;
        -b|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Internal error"
            exit 1
            ;;
    esac
done


if [[ ! " ${VALID_BUILD_TYPES[*]} " =~ " ${BUILD_TYPE} " ]]; then
    echo "Error: Invalid build type '${BUILD_TYPE}'"
    echo "Valid build types are: ${VALID_BUILD_TYPES[*]}"
    exit 1
fi

echo "Configuring Project"
echo "  Toolchain Dir : ${ARM_TOOLCHAIN_DIR}"
echo "  Build Type    : ${BUILD_TYPE}"

cmake -B build \
      -G "Ninja" \
      -DARM_TOOLCHAIN_DIR="${ARM_TOOLCHAIN_DIR}" \
      -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

echo "Building Project"
cmake --build build
