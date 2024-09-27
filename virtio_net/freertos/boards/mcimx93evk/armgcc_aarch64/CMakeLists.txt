# SOC DEVICE SETTING
SET(MCUX_DEVICE "MIMX9352_ca55")

# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.1.1)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# ENABLE DRIVER CACHE CONTROL
ADD_DEFINITIONS(-DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

SET(BoardName "mcimx93evk")

SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
SET(ProjRootPath "${ProjDirPath}/../../../../../")
SET(SdkDirPath "${ProjRootPath}/../mcux-sdk")
SET(RtosPath "${ProjRootPath}/../FreeRTOS-Kernel")
SET(HtMulticorePath "${ProjRootPath}/../heterogeneous-multicore")
SET(BoardPath "${ProjRootPath}/common/freertos/boards/${BoardName}")
SET(CommonPath "${ProjRootPath}/common")
SET(LibsPath "${CommonPath}/libs")
SET(AppPath "${ProjRootPath}/virtio_net")
SET(VirtioNetPath "${HtMulticorePath}/net")
SET(VirtioMailboxPath "${HtMulticorePath}/virtio/mailbox")
SET(VirtioPlatPortsPath "${HtMulticorePath}/virtio/plat_ports")
SET(VirtioIncludePath "${HtMulticorePath}/include")
SET(VirtioDevPath "${HtMulticorePath}/virtio/devices")
SET(CommonBoardPath "${ProjDirPath}/../../../../../common/boards/${BoardName}")
SET(RtosAbstractionLayerPath "${ProjRootPath}/../rtos-abstraction-layer/freertos")

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

project(virtio_net)

set(MCUX_SDK_PROJECT_NAME virtio_net.elf)

include(${BoardPath}/armgcc_aarch64/flags.cmake)
add_executable(${MCUX_SDK_PROJECT_NAME}
    "${AppPath}/freertos/main.c"
    "${BoardPath}/board.c"
    "${BoardPath}/mmu.c"
    "${AppPath}/common/boards/${BoardName}/clock_config.c"
    "${AppPath}/common/boards/${BoardName}/pin_mux.c"
    "${AppPath}/common/boards/${BoardName}/virtio_board.c"
    "${CommonBoardPath}/clock_init.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${BoardPath}
    ${AppBoardPath}
    ${SdkDirPath}/devices/MIMX9352
    ${AppPath}/common/boards/${BoardName}
    ${CommonBoardPath}
    ${CommonPath}
    ${CommonPath}/freertos
    ${CommonPath}/libs
    ${VirtioIncludePath}
)

set(CMAKE_MODULE_PATH
    ${CommonPath}/freertos
    ${CommonPath}/freertos/core/armv8a
    ${CommonPath}/libs/hlog
    ${HtMulticorePath}/libs/gen_sw_mbox
    ${RtosPath}
    ${SdkDirPath}
    ${VirtioMailboxPath}
    ${VirtioNetPath}
    ${VirtioDevPath}
    ${VirtioPlatPortsPath}
    ${RtosAbstractionLayerPath}
)

# include entry for cmake components

set(RTOS_ABSTRACTION_LAYER_TARGET ${MCUX_SDK_PROJECT_NAME})
include(rtos_abstraction_layer)

include(all_devices)

# include modules

include(device_system_ca55)

include(common_freertos)
include(common_freertos_core_armv8a)

include(component_lpuart_adapter)
include(component_lists)
include(component_serial_manager)
include(component_serial_manager_uart)

include(driver_cache_armv8a)
include(driver_clock)
include(driver_common)
include(driver_enet)
include(driver_lpuart)
include(driver_phy-common)
include(driver_phy-device-rtl8211f)

include(middleware_freertos-kernel_extension)
include(middleware_freertos-kernel_heap_4)
include(middleware_freertos-kernel_aarch64)

include(utility_assert)
include(utility_debug_console)
include(utilities_misc_utilities_aarch64)

include(lib_hlog)

include(lib_net_switch)
include(lib_port_enet_freertos)
include(lib_switch_hal_freertos)

include(lib_virtio_net)

include(generic_software_mailbox)
include(plat_ports_ca)

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${HtMulticorePath}/os/freertos/os/mqueue.c
    ${HtMulticorePath}/os/freertos/os/stdio.c
)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/virtio_net.bin)
