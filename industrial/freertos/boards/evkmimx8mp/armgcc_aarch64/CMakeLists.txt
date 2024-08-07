# SOC DEVICE SETTING
SET(MCUX_DEVICE "MIMX8ML8_ca53")

# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.12.0)

project(industrial)

set(MCUX_SDK_PROJECT_NAME ${CMAKE_PROJECT_NAME}.elf)

# config to select component, the format is CONFIG_USE_${component}
set(CONFIG_USE_middleware_freertos-kernel_MIMX8ML8 true)
set(CONFIG_USE_middleware_multicore_rpmsg_lite_freertos true)
ADD_DEFINITIONS(-DAARCH64=1)

# ENABLE DRIVER CACHE CONTROL
ADD_DEFINITIONS(-DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

# ENABLE GEN_AVB
SET(CONFIG_USE_GENAVB ON)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

SET(BoardName "evkmimx8mp")

SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
SET(SdkDirPath "${ProjDirPath}/../../../../../../mcux-sdk")
SET(RtosPath "${ProjDirPath}/../../../../../../FreeRTOS-Kernel")
SET(BoardPath "${ProjDirPath}/../../../../../common/freertos/boards/${BoardName}")
SET(CommonPath "${ProjDirPath}/../../../../../common")
SET(CommonBoardPath "${ProjDirPath}/../../../../../common/boards/${BoardName}")
SET(AppPath "${ProjDirPath}/../../../..")
SET(AppBoardPath "${AppPath}/freertos/boards/${BoardName}")
SET(SdkBoardPath "${SdkDirPath}/devices/MIMX8ML8")
SET(MulticorePath "${ProjDirPath}/../../../../../../middleware/multicore")

SET(ProjRootPath "${ProjDirPath}/../../../../../")
SET(HtMulticorePath "${ProjRootPath}/../heterogeneous-multicore")
SET(RtosAbstractionLayerPath "${ProjRootPath}/../rtos-abstraction-layer/freertos")

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

add_executable(${MCUX_SDK_PROJECT_NAME})

if(CONFIG_USE_GENAVB)
  include(genavb_sdk.cmake)
endif()

include(${BoardPath}/armgcc_aarch64/flags.cmake)

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    "${AppPath}/freertos/boards/${BoardName}/stub.c"
    "${AppPath}/freertos/main.c"
    "${AppPath}/common/boards/${BoardName}/clock_config.c"
    "${AppPath}/common/boards/${BoardName}/hardware_ethernet.c"
    "${AppPath}/common/boards/${BoardName}/hardware_flexcan.c"
    "${AppPath}/common/boards/${BoardName}/pin_mux.c"
    "${AppPath}/common/can.c"
    "${AppPath}/common/ethernet.c"
    "${AppPath}/common/industrial.c"
    "${BoardPath}/board.c"
    "${BoardPath}/mmu.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppPath}
    ${AppPath}/avb_tsn/tsn_app
    ${AppPath}/common
    ${AppPath}/common/boards/${BoardName}
    ${AppPath}/common/boards/include
    ${AppPath}/freertos
    ${CommonPath}
    ${CommonPath}/freertos
    ${CommonPath}/libs
    ${CommonBoardPath}
    ${BoardPath}
    ${AppBoardPath}
    ${SdkDirPath}/devices/MIMX8ML8
    ${SdkDirPath}/drivers/enet_qos
)

set(CMAKE_MODULE_PATH
    ${CommonPath}/freertos
    ${CommonPath}/freertos/core/armv8a
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/hlog
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/rpmsg
    ${CommonPath}/libs/stats
    ${HtMulticorePath}/libs/gen_sw_mbox
    ${MulticorePath}
    ${RtosPath}
    ${SdkDirPath}
    ${RtosAbstractionLayerPath}
)

# include entry for cmake components
set(RTOS_ABSTRACTION_LAYER_TARGET ${MCUX_SDK_PROJECT_NAME})
include(rtos_abstraction_layer)

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
include(driver_flexcan)
include(driver_enet_qos)
include(driver_gpt)
include(driver_igpio)
include(driver_iuart)
include(driver_phy-common)
include(driver_phy-device-rtl8211f)

include(lib_ctrl)
include(lib_jailhouse)
include(lib_hlog)
include(lib_rpmsg)
include(lib_stats)

include(middleware_freertos-kernel_extension)
include(middleware_freertos-kernel_heap_4)
include(middleware_freertos-kernel_aarch64)
include(middleware_multicore_rpmsg_lite_aarch64_freertos)
include(middleware_multicore_rpmsg_lite_freertos)
include(middleware_multicore_rpmsg_lite_MIMX8ML8)

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
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.bin)
