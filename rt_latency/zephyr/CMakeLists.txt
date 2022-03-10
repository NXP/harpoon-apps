cmake_minimum_required(VERSION 3.13.1)

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(AppPath "${ProjDirPath}/..")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt_latency)

zephyr_include_directories(
	${AppPath}/common
	${AppPath}/zephyr/include
	${CommonPath}
	${CommonPath}/zephyr/boards
	${CommonPath}/libs/jailhouse
	${CommonPath}/libs/logging
	${CommonPath}/libs/mailbox
	${CommonPath}/libs/ctrl
	${ProjDirPath}
	)

set(MCUX_SDK_PROJECT_NAME app)

set(CMAKE_MODULE_PATH
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/logging
    ${CommonPath}/libs/mailbox
    ${CommonPath}/libs/stats
)

zephyr_compile_definitions(OS_ZEPHYR)

include(lib_stats)
include(lib_jailhouse)
include(lib_logging)
include(lib_mailbox)
include(lib_ctrl)

target_sources(app PRIVATE main.c)
target_sources(app PRIVATE ../common/rt_latency.c)
