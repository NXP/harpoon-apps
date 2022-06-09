cmake_minimum_required(VERSION 3.13.1)

set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
set(CommonPath "${ProjDirPath}/../../common")
set(AppPath "${ProjDirPath}/..")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(industrial)

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
zephyr_compile_definitions(CONFIG_IND_DISABLE_ENET)

include(lib_ctrl)
include(lib_hlog)
include(lib_jailhouse)
include(lib_mailbox)
include(lib_stats)

target_sources(app PRIVATE
	       main.c
	       ${AppPath}/common/boards/${BoardName}/clock_config.c
	       ${AppPath}/common/boards/${BoardName}/hardware_flexcan.c
	       ${AppPath}/common/boards/${BoardName}/pin_mux.c
	       ${AppPath}/common/can.c
	       ${AppPath}/common/industrial.c
	       )
