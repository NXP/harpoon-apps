# SOC DEVICE SETTING
SET(MCUX_DEVICE "MIMX8MM6_ca53")

# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.1.1)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# Select RPMSG as mailbox transport layer
if("${MBOX_TRANSPORT_RPMSG}" STREQUAL "on")
    add_compile_definitions(MBOX_TRANSPORT_RPMSG)
    # config to select component, the format is CONFIG_USE_${component}
    set(CONFIG_USE_middleware_multicore_rpmsg_lite_freertos true)
    set(CONFIG_USE_middleware_freertos-kernel_MIMX8MM6 true)

endif()

# ENABLE DRIVER CACHE CONTROL
ADD_DEFINITIONS(-DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1)

# ENABLE ENET 1588
ADD_DEFINITIONS(-DENET_ENHANCEDBUFFERDESCRIPTOR_MODE=1)

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
SET(BoardPath "${ProjDirPath}/../../../../../common/freertos/boards/${BoardName}")
SET(CommonPath "${ProjDirPath}/../../../../../common")
SET(AppPath "${ProjDirPath}/../../../..")
SET(AppBoardPath "${AppPath}/freertos/boards/${BoardName}")
SET(SdkBoardPath "${SdkDirPath}/devices/MIMX8MM6")
if("${MBOX_TRANSPORT_RPMSG}" STREQUAL "on")
    SET(MulticorePath "${ProjDirPath}/../../../../../../middleware/multicore")
endif()

SET(GenAVBPath "${ProjDirPath}/../../../../../../gen_avb_sdk")
SET(GenAVBBuildPath "${CMAKE_CURRENT_BINARY_DIR}/gen_avb_build")
SET(FREERTOS_SDK ${SdkDirPath})
SET(FREERTOS_DIR ${RtosPath})
SET(FREERTOS_APPS "${ProjDirPath}/../../../../..")
SET(TARGET "freertos_imx8mm_ca53")
SET(CONFIG "endpoint_tsn")
ADD_SUBDIRECTORY(${GenAVBPath} "${GenAVBBuildPath}")

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

# Overwrite unsupported configurations
add_compile_options(-DSERIAL_MODE=0)
add_compile_options(-DBUILD_MOTOR=0)

project(industrial)

if("${MBOX_TRANSPORT_RPMSG}" STREQUAL "on")
    set(BIN_NAME industrial_rpmsg)
else()
    set(BIN_NAME industrial)
endif()

set(MCUX_SDK_PROJECT_NAME ${BIN_NAME}.elf)

include(${BoardPath}/armgcc_aarch64/flags.cmake)
add_executable(${MCUX_SDK_PROJECT_NAME}
    "${AppPath}/avb_tsn/tsn_app/alarm_task.c"
    "${AppPath}/avb_tsn/tsn_app/configs.c"
    "${AppPath}/avb_tsn/tsn_app/cyclic_task.c"
    "${AppPath}/avb_tsn/tsn_app/tsn_task.c"
    "${AppPath}/avb_tsn/tsn_app/tsn_tasks_config.c"
    "${AppPath}/freertos/main.c"
    "${AppPath}/common/boards/${BoardName}/clock_config.c"
    "${AppPath}/common/boards/${BoardName}/hardware_ethernet.c"
    "${AppPath}/common/boards/${BoardName}/pin_mux.c"
    "${AppPath}/common/boards/${BoardName}/stub.c"
    "${AppPath}/common/ethernet.c"
    "${AppPath}/common/ethernet_avb_tsn.c"
    "${AppPath}/common/ethernet_sdk_enet.c"
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
    ${AppPath}/freertos/boards/${BoardName}
    ${CommonPath}
    ${CommonPath}/freertos
    ${CommonPath}/libs
    ${BoardPath}
    ${AppBoardPath}
    ${GenAVBPath}/include
    ${GenAVBPath}/include/genavb
    ${GenAVBPath}/include/freertos
    ${SdkDirPath}/devices/MIMX8MM6
    ${SdkDirPath}/drivers/enet
)

set(CMAKE_MODULE_PATH
    ${CommonPath}/freertos
    ${CommonPath}/freertos/core/armv8a
    ${CommonPath}/libs/avb_tsn
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/hlog
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/mailbox
    ${CommonPath}/libs/stats
    ${CommonPath}/libs/rpmsg
    ${MulticorePath}
    ${RtosPath}
    ${SdkDirPath}
)

add_library(avb-core-lib STATIC IMPORTED)
set_target_properties(avb-core-lib PROPERTIES IMPORTED_LOCATION ${GenAVBBuildPath}/libstack-core.a)

add_library(srp-lib STATIC IMPORTED)
set_target_properties(srp-lib PROPERTIES IMPORTED_LOCATION ${GenAVBBuildPath}/srp)

add_dependencies(${MCUX_SDK_PROJECT_NAME} avb-core-lib stack-freertos srp-lib)

# include entry for cmake components

include(all_devices)

# include modules

include(device_system_ca53)

include(common_freertos)
include(common_freertos_core_armv8a)

include(component_iuart_adapter)
include(component_lists)
include(component_serial_manager)
include(component_serial_manager_uart)

include(driver_cache_armv8a)
include(driver_clock)
include(driver_common)
include(driver_enet)
include(driver_gpt)
include(driver_igpio)
include(driver_iuart)
include(driver_mdio-common)
include(driver_mdio-enet)
include(driver_phy-common)
include(driver_phy-device-ar8031)

include(lib_avb_tsn)
include(lib_ctrl)
include(lib_jailhouse)
include(lib_hlog)
include(lib_mailbox)
include(lib_stats)

include(middleware_freertos-kernel_extension)
include(middleware_freertos-kernel_heap_4)
include(middleware_freertos-kernel_ca53)

if("${MBOX_TRANSPORT_RPMSG}" STREQUAL "on")
    include(lib_rpmsg)
    include(middleware_multicore_rpmsg_lite_imx8mm_ca53_freertos)
    include(middleware_multicore_rpmsg_lite_MIMX8MM6)
    include(middleware_multicore_rpmsg_lite_freertos)
endif()

include(utility_assert)
include(utility_debug_console)
include(utilities_misc_utilities)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(srp-lib INTERFACE stack-freertos)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE avb-core-lib)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE stack-freertos)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${BIN_NAME}.bin)
