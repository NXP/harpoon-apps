#!/bin/sh

# Select codec to be used:
# CODEC_DEVICE_TYPE="-DCODEC_DEVICE=hifiberry" will enable HiFiBerry Codec
CODEC_DEVICE_TYPE="-DCODEC_DEVICE=hifiberry"

if [ -d "CMakeFiles" ];then rm -rf CMakeFiles; fi
if [ -f "Makefile" ];then rm -f Makefile; fi
if [ -f "cmake_install.cmake" ];then rm -f cmake_install.cmake; fi
if [ -f "CMakeCache.txt" ];then rm -f CMakeCache.txt; fi

cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../mcux-sdk/tools/cmake_toolchain_files/armgcc_aarch64.cmake" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=ddr_debug ${CODEC_DEVICE_TYPE} .
make -j $(nproc)
