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

# Cortex-A55/A53 core maximum clock frequency
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX8MM_EVK_A53 SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX8MN_EVK_A53 SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1600000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX8MP_EVK_A53 SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX93_EVK_A55 SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1700000000UL)

include(lib_hlog)

target_sources(app PRIVATE main.c)
