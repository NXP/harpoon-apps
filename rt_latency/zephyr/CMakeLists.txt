cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt_latency)

if(CONFIG_BOARD_MIMX8MM_EVK_A53)
    set(BoardName "evkmimx8mm")
elseif(CONFIG_BOARD_MIMX8MN_EVK_A53)
    set(BoardName "evkmimx8mn")
elseif(CONFIG_BOARD_MIMX8MP_EVK_A53)
    set(BoardName "evkmimx8mp")
elseif(CONFIG_BOARD_MIMX93_EVK_A55)
    set(BoardName "evkmimx93")
endif()

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(CommonBoardPath "${ProjDirPath}/../../common/boards/${BoardName}")
set(AppPath "${ProjDirPath}/..")
set(MulticorePath "${ProjDirPath}/../../../middleware/multicore")

# config to select component, the format is CONFIG_USE_${component}
set(CONFIG_USE_middleware_zephyr-kernel true)
set(CONFIG_USE_middleware_multicore_rpmsg_lite_zephyr true)
ADD_DEFINITIONS(-DAARCH64=1)

zephyr_include_directories(
	${AppPath}/common
	${AppPath}/zephyr/include
	${CommonPath}
	${CommonPath}/zephyr/boards
	${CommonBoardPath}
	${CommonPath}/libs/jailhouse
	${CommonPath}/libs/hlog
	${CommonPath}/libs/mailbox
	${CommonPath}/libs/ctrl
	${ProjDirPath}
	)

set(MCUX_SDK_PROJECT_NAME app)

set(CMAKE_MODULE_PATH
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/hlog
    ${CommonPath}/libs/mailbox
    ${CommonPath}/libs/stats
    ${CommonPath}/libs/rpmsg
    ${MulticorePath}
)

zephyr_compile_definitions(OS_ZEPHYR)

include(lib_stats)
include(lib_jailhouse)
include(lib_hlog)
include(lib_mailbox)
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

target_sources(app PRIVATE main.c)
target_sources(app PRIVATE ../common/rt_latency.c)
