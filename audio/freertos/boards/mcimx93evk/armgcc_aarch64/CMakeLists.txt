# SOC DEVICE SETTING
SET(MCUX_DEVICE "MIMX9352_ca55")

# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.12.0)

project(audio)

set(MCUX_SDK_PROJECT_NAME ${CMAKE_PROJECT_NAME}.elf)

# ENABLE DRIVER CACHE CONTROL
ADD_DEFINITIONS(-DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1)

# config to select component, the format is CONFIG_USE_${component}
set(CONFIG_USE_middleware_freertos-kernel_MIMX9352 true)
set(CONFIG_USE_middleware_multicore_rpmsg_lite_freertos true)
ADD_DEFINITIONS(-DAARCH64=1)
ADD_DEFINITIONS(-DSDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1700000000UL)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

# ENABLE GEN_AVB
SET(CONFIG_USE_GENAVB ON)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

SET(BoardName "mcimx93evk")

SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
SET(SdkDirPath "${ProjDirPath}/../../../../../../mcux-sdk")
SET(RtosPath "${ProjDirPath}/../../../../../../FreeRTOS-Kernel")
SET(BoardPath "${ProjDirPath}/../../../../../common/freertos/boards/${BoardName}")
SET(CommonPath "${ProjDirPath}/../../../../../common")
SET(CommonBoardPath "${ProjDirPath}/../../../../../common/boards/${BoardName}")
SET(AppPath "${ProjDirPath}/../../../..")
SET(AppBoardPath "${AppPath}/freertos/boards/${BoardName}")
SET(SdkBoardPath "${SdkDirPath}/devices/MIMX9352")
SET(MulticorePath "${ProjDirPath}/../../../../../../middleware/multicore")

SET(ProjRootPath "${ProjDirPath}/../../../../../")
SET(HtMulticorePath "${ProjRootPath}/../heterogeneous-multicore")
SET(RtosAbstractionLayerPath "${ProjRootPath}/../rtos-abstraction-layer/freertos")

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

add_compile_options(-DCODEC_MULTI_ADAPTERS=1)
add_compile_options(-DCODEC_CS42448)
add_compile_options(-DCODEC_CS42448_ENABLE=1)
add_compile_options(-DCODEC_WM8962)
add_compile_options(-DCODEC_WM8962_ENABLE=1)

add_compile_options(-DCONFIG_GENAVB_ENABLE=1)

add_executable(${MCUX_SDK_PROJECT_NAME})

if(CONFIG_USE_GENAVB)
  include(genavb_sdk.cmake)
endif()

include(${BoardPath}/armgcc_aarch64/flags.cmake)

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    "${AppPath}/freertos/main.c"
    "${AppPath}/common/sai_drv.c"
    "${BoardPath}/board.c"
    "${BoardPath}/mmu.c"
    "${AppPath}/common/boards/${BoardName}/clock_config.c"
    "${AppPath}/common/boards/${BoardName}/pin_mux.c"
    "${AppPath}/common/boards/${BoardName}/sai_config.c"
    "${AppPath}/common/boards/${BoardName}/sai_clock_config.c"
    "${AppPath}/common/boards/${BoardName}/codec_config.c"
    "${AppPath}/common/audio.c"
    "${AppPath}/common/audio_app.c"
    "${AppPath}/common/audio_buffer.c"
    "${AppPath}/common/audio_element.c"
    "${AppPath}/common/audio_element_dtmf.c"
    "${AppPath}/common/audio_element_pll.c"
    "${AppPath}/common/audio_element_routing.c"
    "${AppPath}/common/audio_element_sai_sink.c"
    "${AppPath}/common/audio_element_sai_source.c"
    "${AppPath}/common/audio_element_sine.c"
    "${AppPath}/common/audio_pipeline.c"
    "${AppPath}/common/pipeline_config_single_sai.c"
    "${AppPath}/common/play_pipeline.c"
    "${CommonBoardPath}/clock_init.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppPath}
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
    ${SdkDirPath}/devices/MIMX9352
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

include(device_system_ca55)

include(common_freertos)
include(common_freertos_core_armv8a)

include(lib_ctrl)
include(lib_hlog)
include(lib_jailhouse)
include(lib_rpmsg)
include(lib_stats)

include(component_codec_adapters)
include(component_codec_i2c_MIMX9352)
include(component_lists)
include(component_lpuart_adapter)
include(component_serial_manager)
include(component_serial_manager_uart)
include(component_codec_adapters)
include(component_cs42448_adapter)
include(component_wm8962_adapter)

include(driver_cache_armv8a)
include(driver_clock)
include(driver_codec)
include(driver_common)
include(driver_enet_qos)
include(driver_phy-common)
include(driver_phy-device-rtl8211f)
include(driver_lpuart)
include(driver_rgpio)
include(driver_tpm)
include(driver_sai)
include(driver_wm8962)
include(middleware_freertos-kernel_extension)
include(middleware_freertos-kernel_heap_4)
include(middleware_freertos-kernel_aarch64)
include(middleware_multicore_rpmsg_lite_aarch64_freertos)
include(middleware_multicore_rpmsg_lite_freertos)
include(middleware_multicore_rpmsg_lite_MIMX9352)

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
