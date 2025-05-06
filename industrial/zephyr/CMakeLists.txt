cmake_minimum_required(VERSION 3.13.1)

set(TOOLCHAIN_HAS_NEWLIB y)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(industrial)

set(MCUX_SDK_PROJECT_NAME app)

if(CONFIG_BOARD_IMX8MM_EVK)
  set(BoardName "evkmimx8mm")
  set(CONFIG_USE_INDUSTRIAL_CAN OFF)
  set(CONFIG_USE_INDUSTRIAL_ETHERNET ON)
  set(CONFIG_USE_GENAVB ON)
elseif(CONFIG_BOARD_IMX8MN_EVK)
  set(BoardName "evkmimx8mn")
  set(CONFIG_USE_INDUSTRIAL_CAN OFF)
  set(CONFIG_USE_INDUSTRIAL_ETHERNET ON)
  set(CONFIG_USE_GENAVB ON)
elseif(CONFIG_BOARD_IMX8MP_EVK)
  set(BoardName "evkmimx8mp")
  set(CONFIG_USE_INDUSTRIAL_CAN ON)
  set(CONFIG_USE_INDUSTRIAL_ETHERNET ON)
  set(CONFIG_USE_GENAVB ON)
elseif(CONFIG_BOARD_IMX93_EVK)
  set(BoardName "mcimx93evk")
  set(CONFIG_USE_INDUSTRIAL_CAN ON)
  set(CONFIG_USE_INDUSTRIAL_ETHERNET ON)
  set(CONFIG_USE_GENAVB ON)
elseif(CONFIG_BOARD_IMX95_EVK_15X15)
  set(BoardName "imx95lp4xevk15")
  set(CONFIG_USE_INDUSTRIAL_CAN ON)
  set(CONFIG_USE_INDUSTRIAL_ETHERNET OFF)
  set(CONFIG_USE_GENAVB OFF)
elseif(CONFIG_BOARD_IMX95_EVK)
  set(BoardName "imx95lpd5evk19")
  set(CONFIG_USE_INDUSTRIAL_CAN ON)
  set(CONFIG_USE_INDUSTRIAL_ETHERNET OFF)
  set(CONFIG_USE_GENAVB OFF)
else()
  message(FATAL_ERROR "unsupported board")
endif()

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(CommonBoardPath "${ProjDirPath}/../../common/boards/${BoardName}")
set(AppPath "${ProjDirPath}/..")
SET(AppBoardPath "${AppPath}/freertos/boards/${BoardName}")
SET(ProjRootPath "${ProjDirPath}/../../")
SET(RtosAbstractionLayerPath "${ProjRootPath}/../rtos-abstraction-layer/zephyr")

if(CONFIG_USE_GENAVB)
  include(genavb_sdk.cmake)
endif()

zephyr_include_directories(
	${AppPath}/common
	${AppPath}/common/boards/include
	${AppPath}/common/boards/${BoardName}
	${AppPath}/zephyr/boards/${BoardName}
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
zephyr_compile_definitions(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)

# Cortex-A55/A53 core maximum clock frequency
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MM_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MN_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1600000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MP_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX93_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1700000000UL)

include(lib_stats)
include(lib_jailhouse)
include(lib_hlog)
include(lib_ctrl)
include(lib_rpmsg_zephyr)

target_sources(app PRIVATE
	       main.c
	       ${AppPath}/common/boards/${BoardName}/clock_config.c
	       ${AppPath}/common/boards/${BoardName}/pin_mux.c
	       ${AppPath}/common/industrial.c
	       ${AppPath}/zephyr/boards/${BoardName}/app_mmu.c
	       ${AppPath}/zephyr/boards/${BoardName}/stub.c
	       )
if(CONFIG_USE_INDUSTRIAL_CAN)
  target_sources(app PRIVATE
	        ${AppPath}/common/boards/${BoardName}/hardware_flexcan.c
	        ${AppPath}/common/can.c
	      )
endif()
if(CONFIG_USE_INDUSTRIAL_ETHERNET)
  target_sources(app PRIVATE
	        ${AppPath}/common/boards/${BoardName}/hardware_ethernet.c
	        ${AppPath}/common/ethernet.c
	      )
endif()
if(CONFIG_BOARD_IMX8MP_EVK)
  target_sources(app PRIVATE
	       ${AppPath}/common/ethernet_sdk_enet_qos.c
	      )
elseif(CONFIG_BOARD_IMX93_EVK)
  target_sources(app PRIVATE
	       ${AppPath}/common/ethernet_sdk_enet_qos.c
	       ${CommonBoardPath}/clock_init.c
	      )
elseif(CONFIG_BOARD_IMX95_EVK_15X15)
  target_sources(app PRIVATE
	       ${AppPath}/zephyr/boards/${BoardName}/clock_setup.c
	       ${AppPath}/zephyr/boards/${BoardName}/pinctrl.c
	      )
elseif(CONFIG_BOARD_IMX95_EVK)
  target_sources(app PRIVATE
	       ${AppPath}/zephyr/boards/${BoardName}/clock_setup.c
	       ${AppPath}/zephyr/boards/${BoardName}/pinctrl.c
	      )
else()
  target_sources(app PRIVATE
	       ${AppPath}/common/ethernet_sdk_enet.c
	      )
endif()
