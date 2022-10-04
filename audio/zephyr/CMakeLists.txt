cmake_minimum_required(VERSION 3.13.1)

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(AppPath "${ProjDirPath}/..")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(audio)

if(CONFIG_BOARD_MIMX8MP_EVK_A53)
  set(BoardName "evkmimx8mp")
elseif(CONFIG_BOARD_MIMX8MM_EVK_A53)
  set(BoardName "evkmimx8mm")
elseif(CONFIG_BOARD_MIMX8MN_EVK_A53)
  set(BoardName "evkmimx8mn")
else()
  message(FATAL_ERROR "unsupported board")
endif()

zephyr_include_directories(
	${AppPath}/common
	${AppPath}/common/boards/include
	${AppPath}/common/boards/${BoardName}
	${AppPath}/zephyr/include
	${CommonPath}
	${CommonPath}/libs/ctrl
	${CommonPath}/libs/hlog
	${CommonPath}/libs/jailhouse
	${CommonPath}/libs/mailbox
	${CommonPath}/libs
	${CommonPath}/libs/stats
	${CommonPath}/zephyr/boards
	${ProjDirPath}
	)

set(MCUX_SDK_PROJECT_NAME app)

set(CMAKE_MODULE_PATH
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/hlog
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/mailbox
    ${CommonPath}/libs/stats
)

zephyr_compile_definitions(OS_ZEPHYR)
zephyr_compile_definitions(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
zephyr_compile_definitions(CONFIG_AUD_DISABLE_ENET)
# HifiBerry Codec
zephyr_compile_definitions(CODEC_MULTI_ADAPTERS=1)
zephyr_compile_definitions_ifdef(CONFIG_CODEC_PCM512X CODEC_PCM512X_ENABLE=1)
zephyr_compile_definitions_ifdef(CONFIG_CODEC_PCM186X CODEC_PCM186X_ENABLE=1)
# EVK onboard Codec WM8960
zephyr_compile_definitions_ifdef(CONFIG_CODEC_WM8960 CODEC_WM8960)
zephyr_compile_definitions_ifdef(CONFIG_CODEC_WM8960 CODEC_WM8960_ENABLE=1)
# EVK onboard Codec WM8524
zephyr_compile_definitions_ifdef(CONFIG_CODEC_WM8524 CODEC_WM8524)
zephyr_compile_definitions_ifdef(CONFIG_CODEC_WM8524 CODEC_WM8524_ENABLE=1)

include(lib_ctrl)
include(lib_hlog)
include(lib_jailhouse)
include(lib_mailbox)
include(lib_stats)

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
	       ${AppPath}/common/pipeline_config.c
	       ${AppPath}/common/play_pipeline.c
	       ${AppPath}/common/sai_drv.c
	       )
