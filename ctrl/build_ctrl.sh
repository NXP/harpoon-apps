#!/bin/sh

cmake_clean ()
{
	if [ -d "CMakeFiles" ];then rm -rf CMakeFiles; fi
	if [ -f "Makefile" ];then rm -f Makefile; fi
	if [ -f "cmake_install.cmake" ];then rm -f cmake_install.cmake; fi
	if [ -f "CMakeCache.txt" ];then rm -f CMakeCache.txt; fi
}

cmake_clean
cmake -G "Unix Makefiles" -DMBOX_TRANSPORT_RPMSG=off .
make -j $(nproc)

cmake_clean
cmake -G "Unix Makefiles" -DMBOX_TRANSPORT_RPMSG=on .
make -j $(nproc)
