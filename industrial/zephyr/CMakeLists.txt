cmake_minimum_required(VERSION 3.13.1)

set(TOOLCHAIN_HAS_NEWLIB y)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(industrial)

set(MCUX_SDK_PROJECT_NAME app)

if(CONFIG_BOARD_MIMX8MM_EVK_A53)
  set(BoardName "evkmimx8mm")
  set(CONFIG_USE_GENAVB ON)
elseif(CONFIG_BOARD_MIMX8MN_EVK_A53)
  set(BoardName "evkmimx8mn")
  set(CONFIG_USE_GENAVB ON)
elseif(CONFIG_BOARD_MIMX8MP_EVK_A53)
  set(BoardName "evkmimx8mp")
  set(CONFIG_USE_GENAVB ON)
elseif(CONFIG_BOARD_MIMX93_EVK_A55)
  set(BoardName "mcimx93evk")
  set(CONFIG_USE_GENAVB ON)
else()
  message(FATAL_ERROR "unsupported board")
endif()

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(CommonBoardPath "${ProjDirPath}/../../common/boards/${BoardName}")
set(AppPath "${ProjDirPath}/..")
SET(AppBoardPath "${AppPath}/freertos/boards/${BoardName}")
set(MulticorePath "${ProjDirPath}/../../../middleware/multicore")

SET(ProjRootPath "${ProjDirPath}/../../")
SET(HtMulticorePath "${ProjRootPath}/../heterogeneous-multicore")
SET(RtosAbstractionLayerPath "${ProjRootPath}/../rtos-abstraction-layer/zephyr")

if(CONFIG_USE_GENAVB)
  include(genavb_sdk.cmake)
endif()

# config to select component, the format is CONFIG_USE_${component}
set(CONFIG_USE_middleware_zephyr-kernel true)
set(CONFIG_USE_middleware_multicore_rpmsg_lite_zephyr true)
ADD_DEFINITIONS(-DAARCH64=1)

zephyr_include_directories(
	${AppPath}/common
	${AppPath}/common/boards/include
	${AppPath}/common/boards/${BoardName}
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
    ${HtMulticorePath}/libs/gen_sw_mbox
    ${MulticorePath}
    ${RtosAbstractionLayerPath}
)

set(RTOS_ABSTRACTION_LAYER_TARGET ${MCUX_SDK_PROJECT_NAME})
include(rtos_abstraction_layer)

zephyr_compile_definitions(OS_ZEPHYR)
zephyr_compile_definitions(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)

# Cortex-A55/A53 core maximum clock frequency
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX8MM_EVK_A53 SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX8MN_EVK_A53 SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1600000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX8MP_EVK_A53 SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX93_EVK_A55 SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1700000000UL)

include(lib_stats)
include(lib_jailhouse)
include(lib_hlog)
include(lib_ctrl)
include(lib_rpmsg)

include(middleware_multicore_rpmsg_lite_aarch64_zephyr)
include(middleware_multicore_rpmsg_lite_zephyr)

if(CONFIG_BOARD_MIMX8MM_EVK_A53)
  include(middleware_multicore_rpmsg_lite_MIMX8MM6)
elseif(CONFIG_BOARD_MIMX8MN_EVK_A53)
  include(middleware_multicore_rpmsg_lite_MIMX8MN6)
elseif(CONFIG_BOARD_MIMX8MP_EVK_A53)
  include(middleware_multicore_rpmsg_lite_MIMX8ML8)
elseif(CONFIG_BOARD_MIMX93_EVK_A55)
  include(middleware_multicore_rpmsg_lite_MIMX9352)
else()
  message(FATAL_ERROR "unsupported board")
endif()

target_sources(app PRIVATE
	       main.c
	       ${AppPath}/common/boards/${BoardName}/clock_config.c
	       ${AppPath}/common/boards/${BoardName}/hardware_ethernet.c
	       ${AppPath}/common/boards/${BoardName}/pin_mux.c
	       ${AppPath}/common/ethernet.c
	       ${AppPath}/common/industrial.c
	       ${AppPath}/zephyr/boards/${BoardName}/stub.c
	       )
if(CONFIG_BOARD_MIMX8MP_EVK_A53)
  target_sources(app PRIVATE
	       ${AppPath}/common/boards/${BoardName}/hardware_flexcan.c
	       ${AppPath}/common/can.c
	       ${AppPath}/common/ethernet_sdk_enet_qos.c
	      )
elseif(CONFIG_BOARD_MIMX93_EVK_A55)
  target_sources(app PRIVATE
	       ${AppPath}/common/boards/${BoardName}/hardware_flexcan.c
	       ${AppPath}/common/can.c
	       ${AppPath}/common/ethernet_sdk_enet_qos.c
	       ${CommonBoardPath}/clock_init.c
	      )
else()
  target_sources(app PRIVATE
	       ${AppPath}/common/ethernet_sdk_enet.c
	      )
endif()
