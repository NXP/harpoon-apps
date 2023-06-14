# SOC DEVICE SETTING
SET(MCUX_DEVICE "MIMX9352_ca55")

# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.1.1)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
SET(SdkDirPath "${ProjDirPath}/../../../../../../mcux-sdk")
SET(RtosPath "${ProjDirPath}/../../../../../../FreeRTOS-Kernel")
SET(BoardPath "${ProjDirPath}/../../../../../common/freertos/boards/mcimx93evk")
SET(CommonPath "${ProjDirPath}/../../../../../common")
SET(AppPath "${ProjDirPath}/../../../..")
SET(AppBoardPath "${AppPath}/freertos/boards/mcimx93evk")

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

project(hello_world)

set(MCUX_SDK_PROJECT_NAME hello_world.elf)

include(${BoardPath}/armgcc_aarch64/flags.cmake)
add_executable(${MCUX_SDK_PROJECT_NAME}
    "${AppBoardPath}/clock_config.c"
    "${AppPath}/freertos/main.c"
    "${AppPath}/src/hello_world.c"
    "${BoardPath}/board.c"
    "${BoardPath}/mmu.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppBoardPath}
    ${AppPath}/freertos
    ${AppPath}/include
    ${BoardPath}
    ${CommonPath}
    ${CommonPath}/freertos
    ${SdkDirPath}/devices/MIMX9532
)

set(CMAKE_MODULE_PATH
    ${CommonPath}/freertos
    ${CommonPath}/freertos/core/armv8a
    ${CommonPath}/libs/hlog
    ${RtosPath}
    ${SdkDirPath}
)

# include entry for cmake components

include(all_devices)

# include modules

include(device_system_ca55)

include(common_freertos)
include(common_freertos_core_armv8a)

include(lib_hlog)

include(component_lpuart_adapter)
include(component_lists)
include(component_serial_manager)
include(component_serial_manager_uart)

include(driver_clock)
include(driver_common)
include(driver_lpuart)

include(middleware_freertos-kernel_extension)
include(middleware_freertos-kernel_heap_4)
include(middleware_freertos-kernel_aarch64)

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
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/hello_world.bin)
