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
elseif(CONFIG_BOARD_IMX95_EVK_15X15)
  set(TARGET "zephyr_imx95_15x15_ca55")
elseif(CONFIG_BOARD_IMX95_EVK)
  set(TARGET "zephyr_imx95_ca55")
endif()

set(CONFIG "endpoint_tsn")

add_subdirectory(${GenAVBPath} "${GenAVBBuildPath}")

# Overwrite unsupported configurations
add_compile_options(-DSERIAL_MODE=0)
add_compile_options(-DBUILD_MOTOR=0)

zephyr_include_directories(
  ${AppPath}/avb_tsn/tsn_app
  ${CommonPath}/zephyr/boards/${BoardName}
  ${CommonPath}/libs
  ${CommonPath}/libs/avb_tsn
  ${GenAVBPath}/include
  ${GenAVBPath}/include/rtos
)

list(APPEND CMAKE_MODULE_PATH
  ${CommonPath}/libs/avb_tsn
)

zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MM_EVK FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MN_EVK FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MM_EVK ENET_ENHANCEDBUFFERDESCRIPTOR_MODE)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MN_EVK ENET_ENHANCEDBUFFERDESCRIPTOR_MODE)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX8MP_EVK FSL_ETH_ENABLE_CACHE_CONTROL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX93_EVK FSL_ETH_ENABLE_CACHE_CONTROL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX95_EVK_15X15 FSL_ETH_ENABLE_CACHE_CONTROL)
zephyr_compile_definitions_ifdef(CONFIG_BOARD_IMX95_EVK FSL_ETH_ENABLE_CACHE_CONTROL)
zephyr_compile_definitions(CONFIG_GENAVB_ENABLE)

if(CONFIG_BOARD_IMX8MM_EVK OR CONFIG_BOARD_IMX8MN_EVK OR CONFIG_BOARD_IMX95_EVK OR CONFIG_BOARD_IMX95_EVK_15X15)
  set(build_motor_controller 0) # motor_control build disabled
elseif(CONFIG_BOARD_IMX8MP_EVK OR CONFIG_BOARD_IMX93_EVK)
  set(build_motor_controller 1) # motor_control build enabled
else()
  message(FATAL_ERROR "unsupported board")
endif()

# Overwrite unsupported configurations
zephyr_compile_definitions(SERIAL_MODE=0)
zephyr_compile_definitions(BUILD_IO_DEVICE=0) # io_device unsupported
zephyr_compile_definitions(BUILD_MOTOR_CONTROLLER=${build_motor_controller})

add_library(avb-core-lib STATIC IMPORTED)
set_target_properties(avb-core-lib PROPERTIES IMPORTED_LOCATION ${GenAVBBuildPath}/libstack-core.a)
add_dependencies(${MCUX_SDK_PROJECT_NAME} avb-core-lib stack-rtos)

include(lib_avb_tsn)

if(build_motor_controller)
  include(${CMAKE_CURRENT_LIST_DIR}/motor_controller.cmake)
endif()

target_sources(app PRIVATE
  ${AppPath}/avb_tsn/tsn_app/configs.c
  ${AppPath}/common/ethernet_avb_tsn.c
  ${CommonPath}/boards/${BoardName}/genavb_sdk.c
)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE avb-core-lib)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE stack-rtos)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)
