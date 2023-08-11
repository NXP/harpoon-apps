# RTOS abstraction layer configuration
set(RTOS_ABSTRACTION_LAYER_DIR "${ProjDirPath}/../../../rtos-abstraction-layer/zephyr")

set(GenAVBPath "${ProjDirPath}/../../../gen_avb_sdk")
set(GenAVBBuildPath "${CMAKE_CURRENT_BINARY_DIR}/gen_avb_build")
set(CMAKE_TOOLCHAIN_FILE "${ProjDirPath}/../../../modules/hal/nxp/tools/cmake_toolchain_files/armgcc_aarch64.cmake")
# We keep FreeRTOS variables for minimal changes and replace the FreeRTOS API with our stubs
set(FREERTOS_SDK "${ProjDirPath}/../../../modules/hal/nxp/mcux/mcux-sdk")
set(FREERTOS_DIR "${RTOS_ABSTRACTION_LAYER_DIR}/zephyr")
set(FREERTOS_APPS "${ProjDirPath}/../../")
set(CONFIG "endpoint_avb")
add_subdirectory(${GenAVBPath} "${GenAVBBuildPath}")

zephyr_include_directories(
# FIXME move common files (dev_itf.h) to common folder and create common board.h files for zephyr
  ${CommonPath}/freertos/boards/${BoardName}
  ${CommonPath}/libs/avb_tsn
  ${GenAVBPath}/include
  ${GenAVBPath}/include/freertos
)

list(APPEND CMAKE_MODULE_PATH
  ${CommonPath}/libs/avb_tsn
)

zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX8MM_EVK_A53 FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX8MN_EVK_A53 FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX8MM_EVK_A53 ENET_ENHANCEDBUFFERDESCRIPTOR_MODE)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX8MN_EVK_A53 ENET_ENHANCEDBUFFERDESCRIPTOR_MODE)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_MIMX8MP_EVK_A53 FSL_ETH_ENABLE_CACHE_CONTROL)
zephyr_compile_definitions(CONFIG_GENAVB_ENABLE)

add_library(avb-core-lib STATIC IMPORTED)

set_target_properties(avb-core-lib PROPERTIES IMPORTED_LOCATION ${GenAVBBuildPath}/libstack-core.a)
add_dependencies(${MCUX_SDK_PROJECT_NAME} avb-core-lib stack-freertos)

include(lib_avb_tsn)

target_sources(app PRIVATE
  ${AppPath}/common/audio_element_avtp_sink.c
  ${AppPath}/common/audio_element_avtp_source.c
  ${AppPath}/common/avb_config.c
  ${AppPath}/common/boards/${BoardName}/avb_hardware.c
)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE avb-core-lib)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE stack-freertos)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)
