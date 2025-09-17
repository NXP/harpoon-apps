cmake_minimum_required(VERSION 3.13.1)

set(TOOLCHAIN_HAS_NEWLIB y)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(audio)

set(MCUX_SDK_PROJECT_NAME app)

if(CONFIG_BOARD_IMX8MM_EVK)
  set(BoardName "evkmimx8mm")
  set(CONFIG_USE_GENAVB ON)
  set(CONFIG_RTOS_APPS_AUDIO_GENAVB_ENABLE ON)
  set(CONFIG_RTOS_APPS_AUDIO_PLL_ENABLE ON)
elseif(CONFIG_BOARD_IMX8MN_EVK)
  set(BoardName "evkmimx8mn")
  set(CONFIG_USE_GENAVB ON)
  set(CONFIG_RTOS_APPS_AUDIO_GENAVB_ENABLE ON)
  set(CONFIG_RTOS_APPS_AUDIO_PLL_ENABLE ON)
elseif(CONFIG_BOARD_IMX8MP_EVK)
  set(BoardName "evkmimx8mp")
  set(CONFIG_USE_GENAVB ON)
  set(CONFIG_RTOS_APPS_AUDIO_GENAVB_ENABLE ON)
  set(CONFIG_RTOS_APPS_AUDIO_PLL_ENABLE ON)
elseif(CONFIG_BOARD_IMX93_EVK)
  set(BoardName "mcimx93evk")
  set(CONFIG_USE_GENAVB ON)
  set(CONFIG_RTOS_APPS_AUDIO_GENAVB_ENABLE ON)
  set(CONFIG_RTOS_APPS_AUDIO_PLL_ENABLE OFF)
else()
  message(FATAL_ERROR "unsupported board")
endif()

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(CommonBoardPath "${ProjDirPath}/../../common/boards/${BoardName}")
set(AppPath "${ProjDirPath}/..")
SET(ProjRootPath "${ProjDirPath}/../../")
SET(HAL_NXP_PATH "${ProjRootPath}/../zsdk/modules/hal/nxp")
SET(SdkRootDirPath "${HAL_NXP_PATH}/mcux/mcux-sdk-ng")
SET(RtosAbstractionLayerPath "${ProjRootPath}/../rtos-abstraction-layer/zephyr")
SET(RtosAppsPath "${ProjRootPath}/../rtos-apps")

include(${CommonPath}/libs/avb_tsn/lib_avb_tsn_stats.cmake)

if(CONFIG_USE_GENAVB)
  include(genavb_sdk.cmake)
endif()

zephyr_include_directories(
	${AppPath}/common
	${AppPath}/common/boards/include
	${AppPath}/common/boards/${BoardName}
	${AppPath}/zephyr/boards/${BoardName}
	${CommonPath}
	${CommonPath}/libs/ctrl
	${CommonPath}/libs/jailhouse
	${CommonPath}/libs/gen_sw_mbox
	${CommonPath}/libs
	${CommonPath}/zephyr/boards
	${CommonBoardPath}
	${ProjDirPath}
	)

set(CMAKE_MODULE_PATH
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/gen_sw_mbox
    ${CommonPath}/libs/rpmsg
    ${RtosAbstractionLayerPath}
)

set(RTOS_ABSTRACTION_LAYER_TARGET ${MCUX_SDK_PROJECT_NAME})
include(rtos_abstraction_layer)

set(RTOS_APPS_TARGET ${MCUX_SDK_PROJECT_NAME})
include(${RtosAppsPath}/rtos_apps_async.cmake)
include(${RtosAppsPath}/rtos_apps_audio.cmake)
include(${RtosAppsPath}/rtos_apps_log.cmake)
include(${RtosAppsPath}/rtos_apps_stats.cmake)

zephyr_compile_definitions(OS_ZEPHYR)
zephyr_compile_definitions(CONFIG_HAS_CORTEX_A)
zephyr_compile_definitions(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
# HifiBerry Codec
zephyr_compile_definitions(CODEC_MULTI_ADAPTERS=1)

# Cortex-A55/A53 core maximum clock frequency
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MM_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MN_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1600000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MP_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX93_EVK SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1700000000UL)

if(NOT CONFIG_USE_GENAVB)
  zephyr_compile_definitions(CONFIG_AUD_DISABLE_ENET)
endif()

include(lib_ctrl)
include(lib_jailhouse)

include(lib_rpmsg)
include(lib_gen_sw_mbox)

function(include_mcux_driver driver)
  set(CONFIG_MCUX_COMPONENT_driver.${driver} ON)
  include(${SdkRootDirPath}/drivers/${driver}/CMakeLists.txt)
endfunction()

if(CONFIG_BOARD_IMX8MM_EVK)
  include_mcux_driver(sai)
  include_mcux_driver(gpt)
  include_mcux_driver(enet)
  include_mcux_driver(ii2c)

  set(CONFIG_MCUX_COMPONENT_driver.codec ON)
  set(CONFIG_MCUX_COMPONENT_component.codec_i2c ON)
  set(CONFIG_MCUX_COMPONENT_component.codec_adapters ON)
  set(CONFIG_MCUX_COMPONENT_component.pcm186x_adapter ON)
  set(CONFIG_MCUX_COMPONENT_component.pcm512x_adapter ON)
  set(CONFIG_MCUX_COMPONENT_component.wm8524_adapter ON)
  set(CONFIG_MCUX_COMPONENT_driver.pcm186x ON )
  set(CONFIG_MCUX_COMPONENT_driver.pcm512x ON)
  set(CONFIG_MCUX_COMPONENT_driver.wm8524 ON)
  include(${SdkRootDirPath}/components/codec/CMakeLists.txt)
  set(CONFIG_MCUX_COMPONENT_component.ii2c_adapter ON)
  set(CONFIG_MCUX_COMPONENT_component.i2c_adapter_interface ON)
  include(${SdkRootDirPath}/components/i2c/CMakeLists.txt)

  set(CONFIG_MCUX_COMPONENT_component.eth_phy_common ON)
  set(CONFIG_MCUX_COMPONENT_component.phyar8031 ON)
  include(${SdkRootDirPath}/components/phy/CMakeLists.txt)
elseif(CONFIG_BOARD_IMX8MN_EVK)
  include_mcux_driver(sai)
  include_mcux_driver(gpt)
  include_mcux_driver(enet)
  include_mcux_driver(ii2c)

  set(CONFIG_MCUX_COMPONENT_driver.codec ON)
  set(CONFIG_MCUX_COMPONENT_component.codec_i2c ON)
  set(CONFIG_MCUX_COMPONENT_component.codec_adapters ON)
  set(CONFIG_MCUX_COMPONENT_component.pcm186x_adapter ON)
  set(CONFIG_MCUX_COMPONENT_component.pcm512x_adapter ON)
  set(CONFIG_MCUX_COMPONENT_component.wm8524_adapter ON)
  set(CONFIG_MCUX_COMPONENT_driver.pcm186x ON )
  set(CONFIG_MCUX_COMPONENT_driver.pcm512x ON)
  set(CONFIG_MCUX_COMPONENT_driver.wm8524 ON)
  include(${SdkRootDirPath}/components/codec/CMakeLists.txt)
  set(CONFIG_MCUX_COMPONENT_component.ii2c_adapter ON)
  set(CONFIG_MCUX_COMPONENT_component.i2c_adapter_interface ON)
  include(${SdkRootDirPath}/components/i2c/CMakeLists.txt)

  set(CONFIG_MCUX_COMPONENT_component.eth_phy_common ON)
  set(CONFIG_MCUX_COMPONENT_component.phyar8031 ON)
  include(${SdkRootDirPath}/components/phy/CMakeLists.txt)
elseif(CONFIG_BOARD_IMX8MP_EVK)
  # compile ARM common drivers
  target_sources(app PRIVATE 
    ${SdkRootDirPath}/drivers/common/fsl_common_arm.c
  )

  include_mcux_driver(sai)
  include_mcux_driver(gpt)
  include_mcux_driver(enet_qos)
  include_mcux_driver(ii2c)

  # include audiomix
  zephyr_include_directories(${SdkRootDirPath}/devices/i.MX/i.MX8MP/MIMX8ML8/drivers)
  target_sources(app PRIVATE ${SdkRootDirPath}/devices/i.MX/i.MX8MP/MIMX8ML8/drivers/fsl_audiomix.c)

  include_mcux_driver(igpio)

  set(CONFIG_MCUX_COMPONENT_driver.codec ON)
  set(CONFIG_MCUX_COMPONENT_component.codec_i2c ON)
  set(CONFIG_MCUX_COMPONENT_component.codec_adapters ON)
  set(CONFIG_MCUX_COMPONENT_component.pcm186x_adapter ON)
  set(CONFIG_MCUX_COMPONENT_component.pcm512x_adapter ON)
  set(CONFIG_MCUX_COMPONENT_component.wm8960_adapter ON)
  set(CONFIG_MCUX_COMPONENT_driver.pcm186x ON )
  set(CONFIG_MCUX_COMPONENT_driver.pcm512x ON)
  set(CONFIG_MCUX_COMPONENT_driver.wm8960 ON)
  include(${SdkRootDirPath}/components/codec/CMakeLists.txt)
  set(CONFIG_MCUX_COMPONENT_component.ii2c_adapter ON)
  set(CONFIG_MCUX_COMPONENT_component.i2c_adapter_interface ON)
  include(${SdkRootDirPath}/components/i2c/CMakeLists.txt)

  set(CONFIG_MCUX_COMPONENT_component.eth_phy_common ON)
  set(CONFIG_MCUX_COMPONENT_component.phyrtl8211f ON)
  include(${SdkRootDirPath}/components/phy/CMakeLists.txt)
elseif(CONFIG_BOARD_IMX93_EVK)
  # compile ARM common drivers
  target_sources(app PRIVATE 
    ${SdkRootDirPath}/drivers/common/fsl_common_arm.c
  )

  include_mcux_driver(sai)
  include_mcux_driver(tpm)
  include_mcux_driver(enet_qos)
  include_mcux_driver(lpi2c)

  include_mcux_driver(rgpio)

  set(CONFIG_MCUX_COMPONENT_driver.codec ON)
  set(CONFIG_MCUX_COMPONENT_component.codec_i2c ON)
  set(CONFIG_MCUX_COMPONENT_component.codec_adapters ON)
  set(CONFIG_MCUX_COMPONENT_component.cs42448_adapter ON)
  set(CONFIG_MCUX_COMPONENT_component.wm8962_adapter ON)
  set(CONFIG_MCUX_COMPONENT_driver.cs42448 ON )
  set(CONFIG_MCUX_COMPONENT_driver.wm8962 ON)
  include(${SdkRootDirPath}/components/codec/CMakeLists.txt)
  set(CONFIG_MCUX_COMPONENT_component.lpi2c_adapter ON)
  set(CONFIG_MCUX_COMPONENT_component.i2c_adapter_interface ON)
  include(${SdkRootDirPath}/components/i2c/CMakeLists.txt)

  set(CONFIG_MCUX_COMPONENT_component.eth_phy_common ON)
  set(CONFIG_MCUX_COMPONENT_component.phyrtl8211f ON)
  include(${SdkRootDirPath}/components/phy/CMakeLists.txt)
endif()

target_sources(app PRIVATE
	       main.c
	       boards/${BoardName}/app_mmu.c
	       ${AppPath}/common/audio_app.c
	       ${AppPath}/common/boards/${BoardName}/clock_config.c
	       ${AppPath}/common/boards/${BoardName}/codec_config.c
	       ${AppPath}/common/boards/${BoardName}/pin_mux.c
	       ${AppPath}/common/boards/${BoardName}/sai_clock_config.c
	       ${AppPath}/common/boards/${BoardName}/sai_config.c
	       )

if(CONFIG_BOARD_IMX8MM_EVK OR CONFIG_BOARD_IMX8MN_EVK OR CONFIG_BOARD_IMX8MP_EVK)
target_sources(app PRIVATE
	       ${AppPath}/common/pipeline_config.c
               )
elseif(CONFIG_BOARD_IMX93_EVK)
target_sources(app PRIVATE
	       ${AppPath}/common/pipeline_config_single_sai.c
	       ${CommonBoardPath}/clock_init.c
               )
endif()
