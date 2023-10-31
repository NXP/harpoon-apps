cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(audio)

set(MCUX_SDK_PROJECT_NAME app)

if(CONFIG_BOARD_MIMX8MM_EVK_A53)
  set(BoardName "evkmimx8mm")
  set(TARGET "zephyr_imx8mm_ca53")
  if(NOT CONFIG_SMP)
    set(CONFIG_USE_GENAVB ON)
  endif()
elseif(CONFIG_BOARD_MIMX8MN_EVK_A53)
  set(BoardName "evkmimx8mn")
  set(TARGET "zephyr_imx8mn_ca53")
  if(NOT CONFIG_SMP)
    set(CONFIG_USE_GENAVB ON)
  endif()
elseif(CONFIG_BOARD_MIMX8MP_EVK_A53)
  set(BoardName "evkmimx8mp")
  set(TARGET "zephyr_imx8mp_ca53")
  if(NOT CONFIG_SMP)
    set(CONFIG_USE_GENAVB ON)
  endif()
elseif(CONFIG_BOARD_MIMX93_EVK_A55)
  set(BoardName "mcimx93evk")
  set(TARGET "zephyr_imx93_ca55")
else()
  message(FATAL_ERROR "unsupported board")
endif()

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(CommonBoardPath "${ProjDirPath}/../../common/boards/${BoardName}")
set(AppPath "${ProjDirPath}/..")
set(MulticorePath "${ProjDirPath}/../../../middleware/multicore")

SET(ProjRootPath "${ProjDirPath}/../../")
SET(HtMulticorePath "${ProjRootPath}/../heterogeneous-multicore")

if(CONFIG_USE_GENAVB)
  include(genavb.cmake)
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
	${CommonPath}/libs/ctrl
	${CommonPath}/libs/hlog
	${CommonPath}/libs/jailhouse
	${CommonPath}/libs
	${CommonPath}/libs/stats
	${CommonPath}/zephyr/boards
	${CommonBoardPath}
	${ProjDirPath}
	)

set(CMAKE_MODULE_PATH
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/hlog
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/stats
    ${CommonPath}/libs/rpmsg
    ${HtMulticorePath}/libs/gen_sw_mbox
    ${MulticorePath}
)

zephyr_compile_definitions(OS_ZEPHYR)
zephyr_compile_definitions(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
# HifiBerry Codec
zephyr_compile_definitions(CODEC_MULTI_ADAPTERS=1)
zephyr_compile_definitions_ifdef(CONFIG_CODEC_PCM512X CODEC_PCM512X_ENABLE=1)
zephyr_compile_definitions_ifdef(CONFIG_CODEC_PCM186X CODEC_PCM186X_ENABLE=1)
# EVK onboard Codec WM8962
zephyr_compile_definitions_ifdef(CONFIG_CODEC_WM8962 CODEC_WM8962)
zephyr_compile_definitions_ifdef(CONFIG_CODEC_WM8962 CODEC_WM8962_ENABLE=1)
# EVK onboard Codec WM8960
zephyr_compile_definitions_ifdef(CONFIG_CODEC_WM8960 CODEC_WM8960)
zephyr_compile_definitions_ifdef(CONFIG_CODEC_WM8960 CODEC_WM8960_ENABLE=1)
# EVK onboard Codec WM8524
zephyr_compile_definitions_ifdef(CONFIG_CODEC_WM8524 CODEC_WM8524)
zephyr_compile_definitions_ifdef(CONFIG_CODEC_WM8524 CODEC_WM8524_ENABLE=1)

# Cortex-A55 core maximum clock frequency
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX93_EVK_A55 SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1700000000UL)

if(NOT CONFIG_USE_GENAVB)
  zephyr_compile_definitions(CONFIG_AUD_DISABLE_ENET)
endif()

include(lib_ctrl)
include(lib_hlog)
include(lib_jailhouse)
include(lib_stats)
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
	       ${AppPath}/common/audio.c
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

if(CONFIG_BOARD_MIMX8MM_EVK_A53 OR CONFIG_BOARD_MIMX8MN_EVK_A53 OR CONFIG_BOARD_MIMX8MP_EVK_A53)
target_sources(app PRIVATE
	       ${AppPath}/common/audio_pll14xx.c
	       ${AppPath}/common/pipeline_config.c
               )
elseif(CONFIG_BOARD_MIMX93_EVK_A55)
target_sources(app PRIVATE
	       ${AppPath}/common/pipeline_config_single_sai.c
	       ${CommonBoardPath}/clock_init.c
               )
endif()
