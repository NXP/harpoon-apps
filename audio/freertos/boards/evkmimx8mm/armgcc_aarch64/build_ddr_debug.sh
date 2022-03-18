#!/bin/sh

# Select default codec
# Supported values: hifiberry, wm8524
CODEC_PARAMS="-DUSE_CODEC=hifiberry"

if [ -d "CMakeFiles" ];then rm -rf CMakeFiles; fi
if [ -f "Makefile" ];then rm -f Makefile; fi
if [ -f "cmake_install.cmake" ];then rm -f cmake_install.cmake; fi
if [ -f "CMakeCache.txt" ];then rm -f CMakeCache.txt; fi

cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../mcux-sdk/tools/cmake_toolchain_files/armgcc_aarch64.cmake" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=ddr_debug ${CODEC_PARAMS} .
make -j $(nproc)
