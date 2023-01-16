cmake_minimum_required(VERSION 3.13.1)

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(AppPath "${ProjDirPath}/..")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

zephyr_include_directories(
	${AppPath}/common
	${AppPath}/zephyr/include
	${CommonPath}
	${CommonPath}/zephyr/boards
	${CommonPath}/libs/hlog
	${ProjDirPath}
	)

set(MCUX_SDK_PROJECT_NAME app)

set(CMAKE_MODULE_PATH
    ${CommonPath}/libs/hlog
)

zephyr_compile_definitions(OS_ZEPHYR)

include(lib_hlog)

target_sources(app PRIVATE main.c)
