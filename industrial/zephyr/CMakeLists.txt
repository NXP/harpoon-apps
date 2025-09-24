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
  set(CONFIG_APP_MOTOR_CONTROLLER ON)
elseif(CONFIG_BOARD_IMX93_EVK)
  set(BoardName "mcimx93evk")
  set(CONFIG_USE_INDUSTRIAL_CAN ON)
  set(CONFIG_USE_INDUSTRIAL_ETHERNET ON)
  set(CONFIG_USE_GENAVB ON)
  set(CONFIG_APP_MOTOR_CONTROLLER ON)
elseif(CONFIG_BOARD_IMX95_EVK_15X15)
  set(BoardName "imx95lp4xevk15")
  set(CONFIG_USE_INDUSTRIAL_CAN ON)
  set(CONFIG_USE_INDUSTRIAL_ETHERNET ON)
  set(CONFIG_USE_GENAVB ON)
elseif(CONFIG_BOARD_IMX95_EVK)
  set(BoardName "imx95lpd5evk19")
  set(CONFIG_USE_INDUSTRIAL_CAN ON)
  set(CONFIG_USE_INDUSTRIAL_ETHERNET ON)
  set(CONFIG_USE_GENAVB ON)
else()
  message(FATAL_ERROR "unsupported board")
endif()

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(CommonBoardPath "${ProjDirPath}/../../common/boards/${BoardName}")
set(AppPath "${ProjDirPath}/..")
SET(AppBoardPath "${AppPath}/freertos/boards/${BoardName}")
SET(ProjRootPath "${ProjDirPath}/../../")
SET(HAL_NXP_PATH "${ProjRootPath}/../zsdk/modules/hal/nxp")
SET(SdkRootDirPath "${HAL_NXP_PATH}/mcux/mcux-sdk-ng")
SET(RtosAbstractionLayerPath "${ProjRootPath}/../rtos-abstraction-layer/zephyr")
SET(RtosAppsPath "${ProjRootPath}/../rtos-apps")

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
	${CommonPath}/libs/ctrl
	${CommonPath}/libs/gen_sw_mbox
	${ProjDirPath}
	)

set(MCUX_SDK_PROJECT_NAME app)

set(CMAKE_MODULE_PATH
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/rpmsg
    ${CommonPath}/libs/gen_sw_mbox
    ${RtosAbstractionLayerPath}
)

set(RTOS_ABSTRACTION_LAYER_TARGET ${MCUX_SDK_PROJECT_NAME})
include(rtos_abstraction_layer)

set(RTOS_APPS_TARGET ${MCUX_SDK_PROJECT_NAME})
include(${RtosAppsPath}/rtos_apps_async.cmake)
include(${RtosAppsPath}/rtos_apps_log.cmake)
include(${RtosAppsPath}/rtos_apps_stats.cmake)
include(${RtosAppsPath}/rtos_apps_tsn.cmake)

zephyr_compile_definitions(OS_ZEPHYR)
zephyr_compile_definitions(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)

# Cortex-A55/A53 core maximum clock frequency
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MM_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MN_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1600000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MP_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX93_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1700000000UL)

include(lib_jailhouse)
include(lib_ctrl)
include(lib_gen_sw_mbox)
include(lib_rpmsg)

function(include_mcux_driver driver)
  set(CONFIG_MCUX_COMPONENT_driver.${driver} ON)
  include(${SdkRootDirPath}/drivers/${driver}/CMakeLists.txt)
endfunction()

if(CONFIG_BOARD_IMX8MM_EVK)
  # include_mcux_driver(gpt) # already compiled by hal_nxp
  include_mcux_driver(enet)
  
  set(CONFIG_MCUX_COMPONENT_component.eth_phy_common ON)
  set(CONFIG_MCUX_COMPONENT_component.phyar8031 ON)
  include(${SdkRootDirPath}/components/phy/CMakeLists.txt)
elseif(CONFIG_BOARD_IMX8MN_EVK)
  # include_mcux_driver(gpt) # already compiled by hal_nxp
  include_mcux_driver(enet)
  
  set(CONFIG_MCUX_COMPONENT_component.eth_phy_common ON)
  set(CONFIG_MCUX_COMPONENT_component.phyar8031 ON)
  include(${SdkRootDirPath}/components/phy/CMakeLists.txt)
elseif(CONFIG_BOARD_IMX8MP_EVK)
  # compile ARM common drivers
  target_sources(app PRIVATE 
      ${SdkRootDirPath}/drivers/common/fsl_common_arm.c
      )

  # include_mcux_driver(gpt) # already compiled by hal_nxp
  include_mcux_driver(enet_qos)
  include_mcux_driver(flexcan)
  
  include_mcux_driver(igpio)

  set(CONFIG_MCUX_COMPONENT_component.eth_phy_common ON)
  set(CONFIG_MCUX_COMPONENT_component.phyrtl8211f ON)
  include(${SdkRootDirPath}/components/phy/CMakeLists.txt)
elseif(CONFIG_BOARD_IMX93_EVK)
  # compile ARM common drivers
  target_sources(app PRIVATE 
      ${SdkRootDirPath}/drivers/common/fsl_common_arm.c
      )

  # include_mcux_driver(tpm) # already compiled by hal_nxp
  include_mcux_driver(enet_qos)
  include_mcux_driver(flexcan)
  
  include_mcux_driver(rgpio)

  set(CONFIG_MCUX_COMPONENT_component.eth_phy_common ON)
  set(CONFIG_MCUX_COMPONENT_component.phyrtl8211f ON)
  include(${SdkRootDirPath}/components/phy/CMakeLists.txt)
elseif(CONFIG_BOARD_IMX95_EVK_15X15 OR CONFIG_BOARD_IMX95_EVK)
  # compile ARM common drivers
  target_sources(app PRIVATE 
      ${SdkRootDirPath}/drivers/common/fsl_common_arm.c
      )
  # include_mcux_driver(tpm) # already compiled by hal_nxp
  set(CONFIG_MCUX_COMPONENT_driver.netc_imx95 ON)
  include_mcux_driver(netc)
  include_mcux_driver(msgintr)
  include_mcux_driver(flexcan)
  
  include_mcux_driver(rgpio)

  set(CONFIG_MCUX_COMPONENT_component.eth_phy_common ON)
  set(CONFIG_MCUX_COMPONENT_component.phyrtl8211f ON)
  include(${SdkRootDirPath}/components/phy/CMakeLists.txt)
endif()

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
if(CONFIG_BOARD_IMX93_EVK)
  target_sources(app PRIVATE
	       ${CommonBoardPath}/clock_init.c
	      )
elseif(CONFIG_BOARD_IMX95_EVK_15X15)
  target_sources(app PRIVATE
	       ${AppPath}/zephyr/boards/${BoardName}/clock_setup.c
	       ${AppPath}/zephyr/boards/${BoardName}/pinctrl.c
	       ${AppPath}/zephyr/boards/${BoardName}/ethernet_setup.c
	      )
elseif(CONFIG_BOARD_IMX95_EVK)
  target_sources(app PRIVATE
	       ${AppPath}/zephyr/boards/${BoardName}/clock_setup.c
	       ${AppPath}/zephyr/boards/${BoardName}/pinctrl.c
	       ${AppPath}/zephyr/boards/${BoardName}/ethernet_setup.c
	      )
endif()
