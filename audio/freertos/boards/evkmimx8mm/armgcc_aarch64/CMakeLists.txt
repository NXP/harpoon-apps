# SOC DEVICE SETTING
SET(MCUX_DEVICE "MIMX8MM6_ca53")

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

SET(BoardName "evkmimx8mm")

SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
SET(SdkDirPath "${ProjDirPath}/../../../../../../mcux-sdk")
SET(RtosPath "${ProjDirPath}/../../../../../../FreeRTOS-Kernel")
SET(BoardPath "${ProjDirPath}/../../../../../common/freertos/boards/${BoardName}")
SET(CommonPath "${ProjDirPath}/../../../../../common")
SET(AppPath "${ProjDirPath}/../../../..")
SET(SdkBoardPath "${SdkDirPath}/devices/MIMX8MM6")

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

if(CODEC_DEVICE STREQUAL "hifiberry")
  add_compile_options(-DCODEC_MULTI_ADAPTERS=1)
  add_compile_options(-DCODEC_PCM186X_ENABLE=1)
  add_compile_options(-DCODEC_PCM512X_ENABLE=1)
else()
  message(FATAL_ERROR "Please set Codec device")
endif()

project(audio)

set(MCUX_SDK_PROJECT_NAME audio.elf)

include(${BoardPath}/armgcc_aarch64/flags.cmake)
add_executable(${MCUX_SDK_PROJECT_NAME}
    "${AppPath}/freertos/main.c"
    "${AppPath}/common/sai_drv.c"
    "${BoardPath}/board.c"
    "${BoardPath}/mmu.c"
    "${AppPath}/common/boards/${BoardName}/pin_mux.c"
    "${AppPath}/common/boards/${BoardName}/sai_clock_config.c"
    "${AppPath}/common/boards/${BoardName}/sai_codec_config.c"
    "${AppPath}/common/audio_buffer.c"
    "${AppPath}/common/audio_element.c"
    "${AppPath}/common/audio_element_dtmf.c"
    "${AppPath}/common/audio_element_routing.c"
    "${AppPath}/common/audio_element_sai_sink.c"
    "${AppPath}/common/audio_element_sai_source.c"
    "${AppPath}/common/audio_element_sine.c"
    "${AppPath}/common/audio_pipeline.c"
    "${AppPath}/common/dtmf_wave.c"
    "${AppPath}/common/pipeline_config.c"
    "${AppPath}/common/play_dtmf.c"
    "${AppPath}/common/play_music.c"
    "${AppPath}/common/play_pipeline.c"
    "${AppPath}/common/play_sine.c"
    "${AppPath}/common/rec_play.c"
    "${AppPath}/common/rec_play2.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppPath}/include
    ${AppPath}/freertos/boards/${BoardName}
    ${AppPath}/common
    ${CommonPath}
    ${CommonPath}/freertos
    ${BoardPath}
    ${AppPath}/common/boards/include
    ${SdkDirPath}/devices/MIMX8MM6
)

set(CMAKE_MODULE_PATH
    ${CommonPath}/freertos
    ${CommonPath}/freertos/core/armv8a
    ${CommonPath}/libs/jailhouse
    ${CommonPath}/libs/logging
    ${CommonPath}/libs/mailbox
    ${CommonPath}/libs/ctrl
    ${RtosPath}
    ${SdkDirPath}
)

# include entry for cmake components

include(all_devices)

# include modules

include(device_system_ca53)

include(common_freertos)
include(common_freertos_core_armv8a)

include(lib_jailhouse)
include(lib_logging)
include(lib_mailbox)
include(lib_ctrl)

include(component_codec_i2c_MIMX8MM6)
include(component_iuart_adapter)
include(component_lists)
include(component_serial_manager)
include(component_serial_manager_uart)
include(component_codec_adapters)
include(component_pcm186x_adapter)
include(component_pcm512x_adapter)

include(driver_clock)
include(driver_codec)
include(driver_common)
include(driver_iuart)
include(driver_igpio)
include(driver_sai)
include(driver_pcm186x)
include(driver_pcm512x)

include(middleware_freertos-kernel_extension)
include(middleware_freertos-kernel_heap_4)
include(middleware_freertos-kernel_ca53)

include(utility_assert)
include(utility_debug_console)
include(utilities_misc_utilities)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/audio.bin)
