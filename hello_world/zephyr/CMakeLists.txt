cmake_minimum_required(VERSION 3.13.1)

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(AppPath "${ProjDirPath}/..")
set(RtosAbstractionLayerPath "${ProjDirPath}/../../../rtos-abstraction-layer/zephyr")
set(RtosAppsPath "${ProjDirPath}/../../../rtos-apps")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

zephyr_include_directories(
	${AppPath}/common
	${AppPath}/zephyr/include
	${CommonPath}
	${CommonPath}/zephyr/boards
	${ProjDirPath}
	)

set(MCUX_SDK_PROJECT_NAME app)

set(CMAKE_MODULE_PATH
	${RtosAbstractionLayerPath}
)

set(RTOS_ABSTRACTION_LAYER_TARGET ${MCUX_SDK_PROJECT_NAME})
include(rtos_abstraction_layer)

set(RTOS_APPS_TARGET ${MCUX_SDK_PROJECT_NAME})
include(${RtosAppsPath}/rtos_apps_log.cmake)

zephyr_compile_definitions(OS_ZEPHYR)

# Cortex-A55/A53 core maximum clock frequency
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MM_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MN_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1600000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MP_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX93_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1700000000UL)


target_sources(app PRIVATE main.c)
