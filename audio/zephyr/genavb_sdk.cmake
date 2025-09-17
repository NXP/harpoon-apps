# RTOS abstraction layer configuration
set(RTOS_ABSTRACTION_LAYER_DIR "${ProjDirPath}/../../../rtos-abstraction-layer/zephyr")

set(GenAVBPath "${ProjDirPath}/../../../gen_avb_sdk")
set(GenAVBBuildPath "${CMAKE_CURRENT_BINARY_DIR}/gen_avb_build")
# We keep FreeRTOS variables for minimal changes and replace the FreeRTOS API with our stubs
set(MCUX_SDK "${ProjDirPath}/../../../zsdk/modules/hal/nxp/mcux/mcux-sdk-ng")
set(RTOS_DIR "${RTOS_ABSTRACTION_LAYER_DIR}/zephyr")
set(RTOS_APPS "${ProjDirPath}/../../")

if(CONFIG_BOARD_IMX8MM_EVK)
  set(TARGET "zephyr_imx8mm_ca53")
elseif(CONFIG_BOARD_IMX8MN_EVK)
  set(TARGET "zephyr_imx8mn_ca53")
elseif(CONFIG_BOARD_IMX8MP_EVK)
  set(TARGET "zephyr_imx8mp_ca53")
elseif(CONFIG_BOARD_IMX93_EVK)
  set(TARGET "zephyr_imx93_ca55")
endif()

set(CONFIG "endpoint_avb")

add_subdirectory(${GenAVBPath} "${GenAVBBuildPath}")

zephyr_include_directories(
  ${CommonPath}/zephyr/boards/${BoardName}
  ${CommonPath}/libs/avb_tsn
  ${GenAVBPath}/include
  ${GenAVBPath}/include/rtos
)

list(APPEND CMAKE_MODULE_PATH
  ${CommonPath}/libs/avb_tsn
)

# Set ENET driver defines for i.MX 8MM
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MM_EVK FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MM_EVK ENET_ENHANCEDBUFFERDESCRIPTOR_MODE)
# Set ENET driver defines for i.MX 8MN
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MN_EVK FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MN_EVK ENET_ENHANCEDBUFFERDESCRIPTOR_MODE)
# Set ENET_QoS driver defines for i.MX 8MP
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MP_EVK FSL_ETH_ENABLE_CACHE_CONTROL)
# Set ENET_QoS driver defines for i.MX 93
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX93_EVK FSL_ETH_ENABLE_CACHE_CONTROL)

zephyr_compile_definitions(CONFIG_GENAVB_ENABLE)

add_library(avb-core-lib STATIC IMPORTED)

set_target_properties(avb-core-lib PROPERTIES IMPORTED_LOCATION ${GenAVBBuildPath}/libstack-core.a)
add_dependencies(${MCUX_SDK_PROJECT_NAME} avb-core-lib stack-rtos)

include(lib_avb_tsn)

target_sources(app PRIVATE
  ${AppPath}/common/avb_config.c
  ${AppPath}/common/boards/${BoardName}/avb_hardware.c
  ${CommonPath}/boards/${BoardName}/genavb_sdk.c
)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE avb-core-lib)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE stack-rtos)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)
