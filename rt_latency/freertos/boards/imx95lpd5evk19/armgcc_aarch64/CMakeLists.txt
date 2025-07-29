# SOC DEVICE SETTING
set(CONFIG_BOARD imx95lpd5evk19)
set(CONFIG_DEVICE_ID MIMX9596xxxxN)
set(MCUX_DEVICE "MIMX9596_ca55")
# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.12.0)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

SET(BoardName "imx95lpd5evk19")

SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
SET(SdkDirPath "${ProjDirPath}/../../../../../../mcux-sdk")
SET(RtosPath "${ProjDirPath}/../../../../../../FreeRTOS-Kernel")
SET(BoardPath "${ProjDirPath}/../../../../../common/freertos/boards/${BoardName}")
SET(CommonPath "${ProjDirPath}/../../../../../common")
SET(CommonBoardPath "${ProjDirPath}/../../../../../common/boards/${BoardName}")
SET(AppPath "${ProjDirPath}/../../../..")
SET(AppBoardPath "${AppPath}/freertos/boards/${BoardName}")

SET(ProjRootPath "${ProjDirPath}/../../../../../")
SET(RtosAbstractionLayerPath "${ProjRootPath}/../rtos-abstraction-layer/freertos")
SET(RtosAppsPath "${ProjRootPath}/../rtos-apps")

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

project(rt_latency)
set(BIN_NAME rt_latency)

set(MCUX_SDK_PROJECT_NAME ${BIN_NAME}.elf)

include(${BoardPath}/armgcc_aarch64/flags.cmake)
add_executable(${MCUX_SDK_PROJECT_NAME}
    "${AppBoardPath}/clock_config.c"
    "${AppPath}/freertos/main.c"
    "${AppPath}/common/rt_latency.c"
    "${BoardPath}/board.c"
    "${BoardPath}/mmu.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppPath}
    ${AppPath}/common
    ${AppPath}/freertos
    ${AppPath}/include
    ${AppPath}/common/boards/${BoardName}
    ${BoardPath}
    ${CommonPath}
    ${CommonPath}/freertos
    ${CommonBoardPath}
    ${BoardPath}
    ${AppBoardPath}
    ${SdkDirPath}/devices/MIMX9596
)

set(CMAKE_MODULE_PATH
    ${CommonPath}/freertos
    ${CommonPath}/freertos/core/armv8a
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/rpmsg
    ${RtosPath}
    ${SdkDirPath}
    ${RtosAbstractionLayerPath}
)

# set all mcux-sdk flags

set(CONFIG_USE_common_freertos_core_armv8a true)
set(CONFIG_USE_common_freertos true)
set(CONFIG_USE_component_hal_clock_imx95 true)
set(CONFIG_USE_component_hal_clock true)
set(CONFIG_USE_component_hal_pinctrl_imx95 true)
set(CONFIG_USE_component_hal_pinctrl true)
set(CONFIG_USE_component_hal_power_imx95 true)
set(CONFIG_USE_component_hal_power true)
set(CONFIG_USE_component_lists true)
set(CONFIG_USE_component_lpuart_adapter true)
set(CONFIG_USE_component_serial_manager true)
set(CONFIG_USE_component_serial_manager_uart true)
set(CONFIG_USE_component_sm_clock true)
set(CONFIG_USE_component_sm_crc true)
set(CONFIG_USE_component_sm_imx95_ca55 true)
set(CONFIG_USE_component_sm_pinctrl true)
set(CONFIG_USE_component_sm_power true)
set(CONFIG_USE_component_sm_scmi true)
set(CONFIG_USE_component_sm_smt true)
set(CONFIG_USE_device_CMSIS true)
set(CONFIG_USE_device_system_ca55 true)
set(CONFIG_USE_driver_cache_armv8a true)
set(CONFIG_USE_driver_clock true)
set(CONFIG_USE_driver_common true)
set(CONFIG_USE_driver_iomuxc true)
set(CONFIG_USE_driver_lpuart true)
set(CONFIG_USE_driver_mu1 true)
set(CONFIG_USE_middleware_freertos-kernel_aarch64 true)
set(CONFIG_USE_middleware_freertos-kernel_extension true)
set(CONFIG_USE_middleware_freertos-kernel_heap_4 true)

# include entry for cmake components

set(RTOS_ABSTRACTION_LAYER_TARGET ${MCUX_SDK_PROJECT_NAME})
include(rtos_abstraction_layer)

set(RTOS_APPS_TARGET ${MCUX_SDK_PROJECT_NAME})
include(${RtosAppsPath}/rtos_apps_log.cmake)
include(${RtosAppsPath}/rtos_apps_stats.cmake)

include(all_devices)

# include modules

include(device_system_ca55)

include(common_freertos)
include(common_freertos_core_armv8a)

include(driver_counter_tpm)

include(component_lists)
include(component_lpuart_adapter)
include(component_serial_manager)
include(component_serial_manager_uart)

include(driver_cache_armv8a)
include(driver_clock)
include(driver_common)
include(driver_lpuart)
include(driver_tpm)

include(lib_ctrl)
include(lib_jailhouse)
include(lib_rpmsg_freertos)

include(middleware_freertos-kernel_aarch64)
include(middleware_freertos-kernel_extension)
include(middleware_freertos-kernel_heap_4)

include(utilities_misc_utilities_aarch64)
include(utility_assert)
include(utility_debug_console)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
	-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${BIN_NAME}.bin)
