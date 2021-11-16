#!/bin/sh

# Select codec to be used:
# CODEC_DEVICE_TYPE="-DCODEC_DEVICE=wm8960" will enable EVK on board codec
# CODEC_DEVICE_TYPE="-DCODEC_DEVICE=hifiberry" will enable HiFiBerry Codec
CODEC_DEVICE_TYPE="-DCODEC_DEVICE=hifiberry"

do_clean() {
if [ -d "CMakeFiles" ];then rm -rf CMakeFiles; fi
if [ -f "Makefile" ];then rm -f Makefile; fi
if [ -f "cmake_install.cmake" ];then rm -f cmake_install.cmake; fi
if [ -f "CMakeCache.txt" ];then rm -f CMakeCache.txt; fi
}

do_build() {
do_clean
echo "build "$1""
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../mcux-sdk/tools/cmake_toolchain_files/armgcc_aarch64.cmake" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=ddr_debug ${CODEC_DEVICE_TYPE} -DDEMO_TYPE="$1" .
make -j $(nproc)
}

do_build "play_music"
do_build "hifiberry_dtmf"
do_build "hifiberry_record_playback"
do_build "hifiberry_record_playback2"
do_build "hifiberry_sine"
