# SOC DEVICE SETTING
SET(MCUX_DEVICE "MIMX8ML8_ca53")

# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.12.0)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# ENABLE DRIVER CACHE CONTROL
ADD_DEFINITIONS(-DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1)

# Select RPMSG as mailbox transport layer
add_compile_definitions(MBOX_TRANSPORT_RPMSG)

# config to select component, the format is CONFIG_USE_${component}
set(CONFIG_USE_middleware_freertos-kernel_MIMX8ML8 true)
set(CONFIG_USE_middleware_multicore_rpmsg_lite_freertos true)
ADD_DEFINITIONS(-DAARCH64=1)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

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
SET(AppPath "${ProjDirPath}/../../../..")
SET(AppBoardPath "${AppPath}/freertos/boards/${BoardName}")
SET(SdkBoardPath "${SdkDirPath}/devices/MIMX8ML8")
SET(MulticorePath "${ProjDirPath}/../../../../../../middleware/multicore")

SET(GenAVBPath "${ProjDirPath}/../../../../../../gen_avb_sdk")
SET(GenAVBBuildPath "${CMAKE_CURRENT_BINARY_DIR}/gen_avb_build")
SET(FREERTOS_SDK ${SdkDirPath})
SET(FREERTOS_DIR ${RtosPath})
SET(FREERTOS_APPS "${ProjDirPath}/../../../../..")
SET(TARGET "freertos_imx8mp_ca53")
SET(CONFIG "endpoint_avb")
ADD_SUBDIRECTORY(${GenAVBPath} "${GenAVBBuildPath}")

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

add_compile_options(-DCODEC_MULTI_ADAPTERS=1)
add_compile_options(-DCODEC_PCM186X_ENABLE=1)
add_compile_options(-DCODEC_PCM512X_ENABLE=1)
add_compile_options(-DCODEC_WM8960)
add_compile_options(-DCODEC_WM8960_ENABLE=1)
add_compile_options(-DCONFIG_GENAVB_ENABLE=1)

project(audio)
set(BIN_NAME audio)

set(MCUX_SDK_PROJECT_NAME ${BIN_NAME}.elf)

include(${BoardPath}/armgcc_aarch64/flags.cmake)
add_executable(${MCUX_SDK_PROJECT_NAME}
    "${AppPath}/freertos/main.c"
    "${AppPath}/common/sai_drv.c"
    "${BoardPath}/board.c"
    "${BoardPath}/mmu.c"
    "${AppPath}/common/boards/${BoardName}/avb_hardware.c"
    "${AppPath}/common/boards/${BoardName}/clock_config.c"
    "${AppPath}/common/boards/${BoardName}/pin_mux.c"
    "${AppPath}/common/boards/${BoardName}/sai_config.c"
    "${AppPath}/common/boards/${BoardName}/sai_clock_config.c"
    "${AppPath}/common/boards/${BoardName}/codec_config.c"
    "${SdkBoardPath}/drivers/fsl_audiomix.c"
    "${AppPath}/common/audio.c"
    "${AppPath}/common/audio_buffer.c"
    "${AppPath}/common/audio_element.c"
    "${AppPath}/common/audio_element_avtp_sink.c"
    "${AppPath}/common/audio_element_avtp_source.c"
    "${AppPath}/common/audio_element_dtmf.c"
    "${AppPath}/common/audio_element_pll.c"
    "${AppPath}/common/audio_element_routing.c"
    "${AppPath}/common/audio_element_sai_sink.c"
    "${AppPath}/common/audio_element_sai_source.c"
    "${AppPath}/common/audio_element_sine.c"
    "${AppPath}/common/audio_pipeline.c"
    "${AppPath}/common/avb_config.c"
    "${AppPath}/common/pipeline_config.c"
    "${AppPath}/common/play_pipeline.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppPath}
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
    ${GenAVBPath}/include/freertos
    ${SdkDirPath}/devices/MIMX8ML8
    ${SdkDirPath}/drivers/enet_qos
)

set(CMAKE_MODULE_PATH
    ${CommonPath}/freertos
    ${CommonPath}/freertos/core/armv8a
    ${CommonPath}/libs/avb_tsn
    ${CommonPath}/libs/ctrl
    ${CommonPath}/libs/hlog
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/mailbox
    ${CommonPath}/libs/rpmsg
    ${CommonPath}/libs/stats
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

include(lib_avb_tsn)
include(lib_ctrl)
include(lib_hlog)
include(lib_jailhouse)
include(lib_mailbox)
include(lib_rpmsg)
include(lib_stats)

include(component_codec_i2c_MIMX8ML8)
include(component_iuart_adapter)
include(component_lists)
include(component_serial_manager)
include(component_serial_manager_uart)
include(component_codec_adapters)
include(component_wm8960_adapter)
include(component_pcm186x_adapter)
include(component_pcm512x_adapter)

include(driver_cache_armv8a)
include(driver_clock)
include(driver_codec)
include(driver_common)
include(driver_enet_qos)
include(driver_gpt)
include(driver_iuart)
include(driver_igpio)
include(driver_pcm186x)
include(driver_pcm512x)
include(driver_phy-common)
include(driver_phy-device-rtl8211f)
include(driver_sai)
include(driver_wm8960)

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
