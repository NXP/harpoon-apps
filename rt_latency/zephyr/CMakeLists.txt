cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt_latency)

if(CONFIG_BOARD_IMX8MM_EVK)
    set(BoardName "evkmimx8mm")
elseif(CONFIG_BOARD_IMX8MN_EVK)
    set(BoardName "evkmimx8mn")
elseif(CONFIG_BOARD_IMX8MP_EVK)
    set(BoardName "evkmimx8mp")
elseif(CONFIG_BOARD_IMX93_EVK)
    set(BoardName "mcimx93evk")
elseif(CONFIG_BOARD_IMX95_EVK_15X15)
    set(BoardName "imx95lp4xevk15")
elseif(CONFIG_BOARD_IMX95_EVK)
    set(BoardName "imx95lpd5evk19")
endif()

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(CommonBoardPath "${ProjDirPath}/../../common/boards/${BoardName}")
set(AppPath "${ProjDirPath}/..")
set(ProjRootPath "${ProjDirPath}/../../")
set(RtosAbstractionLayerPath "${ProjRootPath}/../rtos-abstraction-layer/zephyr")

zephyr_include_directories(
	${AppPath}/common
	${AppPath}/zephyr/include
	${CommonPath}
	${CommonPath}/zephyr/boards
	${CommonBoardPath}
	${CommonPath}/libs/jailhouse
	${CommonPath}/libs/hlog
	${CommonPath}/libs/ctrl
	${ProjDirPath}
	)

set(MCUX_SDK_PROJECT_NAME app)

set(CMAKE_MODULE_PATH
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/hlog
    ${CommonPath}/libs/stats
    ${CommonPath}/libs/rpmsg
    ${RtosAbstractionLayerPath}
)

set(RTOS_ABSTRACTION_LAYER_TARGET ${MCUX_SDK_PROJECT_NAME})
include(rtos_abstraction_layer)

zephyr_compile_definitions(OS_ZEPHYR)

# Cortex-A55/A53 core maximum clock frequency
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MM_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MN_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1600000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MP_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX93_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1700000000UL)

# include modules

include(lib_stats)
include(lib_jailhouse)
include(lib_hlog)
include(lib_ctrl)
include(lib_rpmsg_zephyr)

target_sources(app PRIVATE main.c)
target_sources(app PRIVATE ../common/rt_latency.c)
