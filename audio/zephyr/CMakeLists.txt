cmake_minimum_required(VERSION 3.13.1)

set(TOOLCHAIN_HAS_NEWLIB y)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(audio)

set(MCUX_SDK_PROJECT_NAME app)

if(CONFIG_BOARD_IMX8MM_EVK)
  set(BoardName "evkmimx8mm")
  set(CONFIG_USE_GENAVB ON)
elseif(CONFIG_BOARD_IMX8MN_EVK)
  set(BoardName "evkmimx8mn")
  set(CONFIG_USE_GENAVB ON)
elseif(CONFIG_BOARD_IMX8MP_EVK)
  set(BoardName "evkmimx8mp")
  set(CONFIG_USE_GENAVB ON)
elseif(CONFIG_BOARD_IMX93_EVK)
  set(BoardName "mcimx93evk")
  set(CONFIG_USE_GENAVB ON)
else()
  message(FATAL_ERROR "unsupported board")
endif()

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(CommonBoardPath "${ProjDirPath}/../../common/boards/${BoardName}")
set(AppPath "${ProjDirPath}/..")
SET(ProjRootPath "${ProjDirPath}/../../")
SET(HAL_NXP_PATH "${ProjRootPath}/../modules/hal/nxp")
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
	${CommonPath}
	${CommonPath}/libs/ctrl
	${CommonPath}/libs/jailhouse
	${CommonPath}/libs
	${CommonPath}/zephyr/boards
	${CommonBoardPath}
	${ProjDirPath}
	)

set(CMAKE_MODULE_PATH
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/rpmsg
    ${RtosAbstractionLayerPath}
)

set(RTOS_ABSTRACTION_LAYER_TARGET ${MCUX_SDK_PROJECT_NAME})
include(rtos_abstraction_layer)

set(RTOS_APPS_TARGET ${MCUX_SDK_PROJECT_NAME})
include(${RtosAppsPath}/rtos_apps_log.cmake)
include(${RtosAppsPath}/rtos_apps_stats.cmake)

zephyr_compile_definitions(OS_ZEPHYR)
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
include(lib_rpmsg_zephyr)

if(CONFIG_BOARD_IMX8MM_EVK)
  set(MCUX_DEVICE "MIMX8MM6_ca53")
  list(APPEND CMAKE_MODULE_PATH
    ${HAL_NXP_PATH}/mcux/mcux-sdk/devices/MIMX8MM6/drivers
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/i2c
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/pcm186x
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/pcm512x
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/wm8524
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/i2c
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/phy
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/phy/device/phyar8031
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/common
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/enet
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/gpt
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/igpio
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/ii2c
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/sai
  )

  include(driver_enet)
  include(driver_gpt)
  include(driver_igpio)
  include(driver_ii2c)
  include(driver_phy-common)
  include(driver_phy-device-ar8031)
  include(driver_sai)

  include(driver_codec)

  include(component_codec_adapters)
  include(component_pcm186x_adapter)
  include(component_pcm512x_adapter)
  include(component_wm8524_adapter)
  include(driver_pcm186x)
  include(driver_pcm512x)
  include(driver_wm8524)

  zephyr_compile_definitions(CODEC_PCM512X_ENABLE=1)
  zephyr_compile_definitions(CODEC_PCM186X_ENABLE=1)
  zephyr_compile_definitions(CODEC_WM8524)
  zephyr_compile_definitions(CODEC_WM8524_ENABLE=1)
elseif(CONFIG_BOARD_IMX8MN_EVK)
  set(MCUX_DEVICE "MIMX8MN6_ca53")
  list(APPEND CMAKE_MODULE_PATH
    ${HAL_NXP_PATH}/mcux/mcux-sdk/devices/MIMX8MN6/drivers
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/i2c
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/pcm186x
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/pcm512x
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/wm8524
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/i2c
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/phy
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/phy/device/phyar8031
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/common
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/enet
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/gpt
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/igpio
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/ii2c
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/sai
  )

  include(driver_enet)
  include(driver_gpt)
  include(driver_igpio)
  include(driver_phy-common)
  include(driver_phy-device-ar8031)
  include(driver_sai)

  include(driver_codec)
  include(driver_ii2c)

  include(component_codec_adapters)
  include(component_pcm186x_adapter)
  include(component_pcm512x_adapter)
  include(component_wm8524_adapter)
  include(driver_pcm186x)
  include(driver_pcm512x)
  include(driver_wm8524)

  zephyr_compile_definitions(CODEC_PCM512X_ENABLE=1)
  zephyr_compile_definitions(CODEC_PCM186X_ENABLE=1)
  zephyr_compile_definitions(CODEC_WM8524)
  zephyr_compile_definitions(CODEC_WM8524_ENABLE=1)
elseif(CONFIG_BOARD_IMX8MP_EVK)
  set(MCUX_DEVICE "MIMX8ML8_ca53")
  list(APPEND CMAKE_MODULE_PATH
    ${HAL_NXP_PATH}/mcux/mcux-sdk/devices/MIMX8ML8/drivers
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/i2c
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/pcm186x
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/pcm512x
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/wm8960
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/i2c
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/phy
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/phy/device/phyrtl8211f
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/phy/mdio/enet_qos
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/common
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/enet_qos
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/flexcan
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/gpt
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/igpio
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/igpio
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/ii2c
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/sai
  )

  include(driver_audiomix)
  include(driver_enet_qos)
  include(driver_flexcan)
  include(driver_gpt)
  include(driver_igpio)
  include(driver_igpio)
  include(driver_mdio-common)
  include(driver_mdio-enet-qos)
  include(driver_phy-common)
  include(driver_phy-device-rtl8211f)
  include(driver_sai)

  include(driver_codec)
  include(driver_ii2c)

  include(component_codec_adapters)
  include(component_pcm186x_adapter)
  include(component_pcm512x_adapter)
  include(component_wm8960_adapter)
  include(driver_pcm186x)
  include(driver_pcm512x)
  include(driver_wm8960)

  zephyr_compile_definitions(CODEC_PCM512X_ENABLE=1)
  zephyr_compile_definitions(CODEC_PCM186X_ENABLE=1)
  zephyr_compile_definitions(CODEC_WM8960)
  zephyr_compile_definitions(CODEC_WM8960_ENABLE=1)
elseif(CONFIG_BOARD_IMX93_EVK)
  set(MCUX_DEVICE "MIMX9352_ca55")
  list(APPEND CMAKE_MODULE_PATH
    ${HAL_NXP_PATH}/mcux/mcux-sdk/devices/MIMX9352/drivers
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/cs42448
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/i2c
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/pcm186x
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/pcm512x
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/codec/wm8962
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/i2c
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/phy
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/phy/device/phyrtl8211f
    ${HAL_NXP_PATH}/mcux/mcux-sdk/components/phy/mdio/enet_qos
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/common
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/enet_qos
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/flexcan
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/lpi2c
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/rgpio
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/sai
    ${HAL_NXP_PATH}/mcux/mcux-sdk/drivers/tpm
  )

  include(driver_enet_qos)
  include(driver_flexcan)
  include(driver_mdio-common)
  include(driver_mdio-enet-qos)
  include(driver_phy-device-rtl8211f)
  include(driver_rgpio)
  include(driver_sai)
  include(driver_tpm)

  include(driver_lpi2c)
  include(driver_codec)

  include(component_codec_adapters)
  include(component_cs42448_adapter)
  include(component_wm8962_adapter)
  include(driver_cs42448)
  include(driver_wm8962)

  zephyr_compile_definitions(CODEC_WM8962)
  zephyr_compile_definitions(CODEC_WM8962_ENABLE=1)
  zephyr_compile_definitions(CODEC_CS42448)
  zephyr_compile_definitions(CODEC_CS42448_ENABLE=1)
endif()

target_sources(app PRIVATE
	       main.c
	       boards/${BoardName}/app_mmu.c
	       ${AppPath}/common/audio.c
	       ${AppPath}/common/audio_app.c
	       ${AppPath}/common/audio_buffer.c
	       ${AppPath}/common/audio_element.c
	       ${AppPath}/common/audio_element_dtmf.c
	       ${AppPath}/common/audio_element_pll.c
	       ${AppPath}/common/audio_element_routing.c
	       ${AppPath}/common/audio_element_sai_sink.c
	       ${AppPath}/common/audio_element_sai_source.c
	       ${AppPath}/common/audio_element_sine.c
	       ${AppPath}/common/audio_pipeline.c
	       ${AppPath}/common/boards/${BoardName}/clock_config.c
	       ${AppPath}/common/boards/${BoardName}/codec_config.c
	       ${AppPath}/common/boards/${BoardName}/pin_mux.c
	       ${AppPath}/common/boards/${BoardName}/sai_clock_config.c
	       ${AppPath}/common/boards/${BoardName}/sai_config.c
	       ${AppPath}/common/play_pipeline.c
	       ${AppPath}/common/sai_drv.c
	       )

if(CONFIG_BOARD_IMX8MM_EVK OR CONFIG_BOARD_IMX8MN_EVK OR CONFIG_BOARD_IMX8MP_EVK)
target_sources(app PRIVATE
	       ${AppPath}/common/audio_pll14xx.c
	       ${AppPath}/common/pipeline_config.c
               )
elseif(CONFIG_BOARD_IMX93_EVK)
target_sources(app PRIVATE
	       ${AppPath}/common/pipeline_config_single_sai.c
	       ${CommonBoardPath}/clock_init.c
               )
endif()
