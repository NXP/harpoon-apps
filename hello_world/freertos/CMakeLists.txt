cmake_minimum_required(VERSION 3.22.0)

# Define CUSTOM_BOARD_ROOT to avoid using mcux-sdk-example project configuration
set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(BOARD_ROOT_ABSOLUTE ${ProjDirPath}/boards REALPATH)
set(CUSTOM_BOARD_ROOT ${BOARD_ROOT_ABSOLUTE})

message(STATUS "CUSTOM_BOARD_ROOT absolute path: ${CUSTOM_BOARD_ROOT}")

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

mcux_set_variable(harpoon_root_path "../../harpoon-apps")
mcux_set_variable(harpoon_os_board_path "${harpoon_root_path}/common/freertos/boards/${board}")
mcux_set_variable(harpoon_app_path "${harpoon_root_path}/hello_world")
mcux_set_variable(harpoon_app_board_path "${harpoon_app_path}/boards/${board}")
mcux_set_variable(harpoon_app_os_board_path "${harpoon_app_path}/freertos/boards/${board}")

project(hello_world LANGUAGES C ASM PROJECT_BOARD_PORT_PATH ${harpoon_app_board_path})

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${harpoon_app_board_path}
             ${harpoon_os_board_path}
             ${harpoon_root_path}/common
             ${harpoon_root_path}/common/freertos
)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${harpoon_os_board_path}/mmu.c
            ${harpoon_os_board_path}/board.c
            ${harpoon_app_path}/src/hello_world.c
)

mcux_add_source(
    SOURCES main.c
)

# Common FreeRTOS configurations
include(${SdkRootDirPath}/${harpoon_root_path}/common/freertos/common_freertos.cmake)
include(${SdkRootDirPath}/${harpoon_root_path}/common/freertos/core/armv8a/common_freertos_core_armv8a.cmake)
include(${SdkRootDirPath}/${harpoon_root_path}/common/libs/jailhouse/lib_jailhouse.cmake)

# Application-specific reconfig
include(${SdkRootDirPath}/${harpoon_app_os_board_path}/reconfig.cmake OPTIONAL)

# Abstraction Layer and Libraries
include(${SdkRootDirPath}/../../rtos-apps/mcux_rtos_apps.cmake)
include(${SdkRootDirPath}/../../rtos-abstraction-layer/freertos/mcux_rtos_abstraction_layer.cmake)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin)
