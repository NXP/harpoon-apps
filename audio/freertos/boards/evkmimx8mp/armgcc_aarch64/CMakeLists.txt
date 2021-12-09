# SOC DEVICE SETTING
SET(MCUX_DEVICE "MIMX8ML8_ca53")

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
SET(BoardPath "${ProjDirPath}/../../../../../common/freertos/boards/evkmimx8mp")
SET(CommonPath "${ProjDirPath}/../../../../../common")
SET(AppPath "${ProjDirPath}/../../../..")
SET(AppBoardPath "${AppPath}/freertos/boards/evkmimx8mp")
SET(SdkBoardPath "${SdkDirPath}/devices/MIMX8ML8")

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

if(CODEC_DEVICE STREQUAL "wm8960")
SET(app_extra_flags "-DCODEC_WM8960")
STRING(APPEND app_extra_flags " -DCODEC_WM8960_ENABLE=1")
elseif(CODEC_DEVICE STREQUAL "hifiberry")
SET(app_extra_flags "-DCODEC_HIFIBERRY")
STRING(APPEND app_extra_flags " -DCODEC_MULTI_ADAPTERS=1")
STRING(APPEND app_extra_flags " -DCODEC_PCM512X_ENABLE=1")
STRING(APPEND app_extra_flags " -DCODEC_PCM186X_ENABLE=1")
else()
message(FATAL_ERROR "Please set Codec device")
endif()

if(DEMO_TYPE STREQUAL "play_dtmf")
  SET(demo_name "play_dtmf")
  STRING(APPEND app_extra_flags " -DDEMO_MODE=0")
elseif(DEMO_TYPE STREQUAL "play_music")
  SET(demo_name "play_music")
  STRING(APPEND app_extra_flags " -DDEMO_MODE=1")
elseif(DEMO_TYPE STREQUAL "play_sine")
  SET(demo_name "play_sine")
  STRING(APPEND app_extra_flags " -DDEMO_MODE=2")
elseif(DEMO_TYPE STREQUAL "rec_play")
  SET(demo_name "rec_play")
  STRING(APPEND app_extra_flags " -DDEMO_MODE=3")
else(DEMO_TYPE STREQUAL "rec_play2")
  SET(demo_name "rec_play2")
  STRING(APPEND app_extra_flags " -DDEMO_MODE=4")
endif()

project(${demo_name})

set(MCUX_SDK_PROJECT_NAME ${demo_name}.elf)

include(${ProjDirPath}/flags.cmake)
add_executable(${MCUX_SDK_PROJECT_NAME}
    "${AppPath}/freertos/main.c"
    "${AppPath}/common/sai_drv.c"
    "${BoardPath}/board.c"
    "${BoardPath}/mmu.c"
    "${AppBoardPath}/sai_clock_config.c"
    "${AppBoardPath}/sai_codec_config.c"
    "${SdkBoardPath}/drivers/fsl_audiomix.c"
    "${AppPath}/common/dtmf_wave.c"
    "${AppPath}/common/play_dtmf.c"
    "${AppPath}/common/play_music.c"
    "${AppPath}/common/play_sine.c"
    "${AppPath}/common/rec_play.c"
    "${AppPath}/common/rec_play2.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppPath}/include
    ${AppPath}/freertos
    ${AppPath}/common
    ${CommonPath}
    ${CommonPath}/freertos
    ${BoardPath}
    ${AppBoardPath}
    ${SdkDirPath}/devices/MIMX8ML8
)

set(CMAKE_MODULE_PATH
    ${CommonPath}/freertos
    ${CommonPath}/freertos/core/armv8a
    ${RtosPath}
    ${SdkDirPath}
)

# include entry for cmake components

include(all_devices)

# include modules

include(common_freertos)
include(common_freertos_core_armv8a)

include(component_codec_i2c_MIMX8ML8)
include(component_iuart_adapter)
include(component_lists)
include(component_serial_manager)
include(component_serial_manager_uart)
if(CODEC_DEVICE STREQUAL "wm8960")
include(component_wm8960_adapter)
elseif(CODEC_DEVICE STREQUAL "hifiberry")
include(component_codec_adapters)
include(component_pcm186x_adapter)
include(component_pcm512x_adapter)
endif()

include(driver_clock)
include(driver_codec)
include(driver_common)
include(driver_iuart)
include(driver_igpio)
include(driver_sai)
if(CODEC_DEVICE STREQUAL "wm8960")
include(driver_wm8960)
elseif(CODEC_DEVICE STREQUAL "hifiberry")
include(driver_pcm186x)
include(driver_pcm512x)
endif()

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
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${demo_name}.bin)
