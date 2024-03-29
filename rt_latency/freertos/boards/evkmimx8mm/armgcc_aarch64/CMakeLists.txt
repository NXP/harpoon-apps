# SOC DEVICE SETTING
SET(MCUX_DEVICE "MIMX8MM6_ca53")

# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.12.0)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# config to select component, the format is CONFIG_USE_${component}
set(CONFIG_USE_middleware_freertos-kernel_MIMX8MM6 true)
set(CONFIG_USE_middleware_multicore_rpmsg_lite_freertos true)
ADD_DEFINITIONS(-DAARCH64=1)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

SET(BoardName "evkmimx8mm")

SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
SET(SdkDirPath "${ProjDirPath}/../../../../../../mcux-sdk")
SET(RtosPath "${ProjDirPath}/../../../../../../FreeRTOS-Kernel")
SET(BoardPath "${ProjDirPath}/../../../../../common/freertos/boards/evkmimx8mm")
SET(CommonPath "${ProjDirPath}/../../../../../common")
SET(CommonBoardPath "${ProjDirPath}/../../../../../common/boards/evkmimx8mm")
SET(AppPath "${ProjDirPath}/../../../..")
SET(AppBoardPath "${AppPath}/freertos/boards/evkmimx8mm")
SET(MulticorePath "${ProjDirPath}/../../../../../../middleware/multicore")

SET(ProjRootPath "${ProjDirPath}/../../../../../")
SET(HtMulticorePath "${ProjRootPath}/../heterogeneous-multicore")

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

project(rt_latency)
set(BIN_NAME rt_latency)

set(MCUX_SDK_PROJECT_NAME ${BIN_NAME}.elf)

include(${BoardPath}/armgcc_aarch64/flags.cmake)
add_executable(${MCUX_SDK_PROJECT_NAME}
    "${AppPath}/freertos/main.c"
    "${AppPath}/common/rt_latency.c"
    "${BoardPath}/board.c"
    "${BoardPath}/mmu.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppPath}
    ${AppPath}/common
    ${AppPath}/freertos
    ${AppPath}/common/boards/${BoardName}
    ${CommonPath}
    ${CommonPath}/freertos
    ${CommonPath}/libs/stats
    ${CommonBoardPath}
    ${BoardPath}
    ${AppBoardPath}
    ${SdkDirPath}/devices/MIMX8MM6
)

set(CMAKE_MODULE_PATH
    ${CommonPath}/freertos
    ${CommonPath}/freertos/core/armv8a
    ${CommonPath}/libs/stats
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/hlog
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/rpmsg
    ${HtMulticorePath}/libs/gen_sw_mbox
    ${MulticorePath}
    ${RtosPath}
    ${SdkDirPath}
)

# include entry for cmake components

include(all_devices)

# include modules

include(device_system_ca53)

include(common_freertos)
include(common_freertos_core_armv8a)

include(driver_counter_gpt)

include(component_iuart_adapter)
include(component_lists)
include(component_serial_manager)
include(component_serial_manager_uart)

include(driver_cache_armv8a)
include(driver_clock)
include(driver_common)
include(driver_gpt)
include(driver_iuart)

include(lib_stats)
include(lib_jailhouse)
include(lib_hlog)
include(lib_ctrl)
include(lib_rpmsg)

include(middleware_freertos-kernel_aarch64)
include(middleware_freertos-kernel_extension)
include(middleware_freertos-kernel_heap_4)
include(middleware_multicore_rpmsg_lite_aarch64_freertos)
include(middleware_multicore_rpmsg_lite_freertos)
include(middleware_multicore_rpmsg_lite_MIMX8MM6)

include(utility_assert)
include(utility_debug_console)
include(utilities_misc_utilities_aarch64)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
	-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${BIN_NAME}.bin)
