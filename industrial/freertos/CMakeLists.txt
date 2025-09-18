cmake_minimum_required(VERSION 3.22.0)

# Define CUSTOM_BOARD_ROOT to avoid using mcux-sdk-example project configuration
set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(BOARD_ROOT_ABSOLUTE ${ProjDirPath}/boards REALPATH)
set(CUSTOM_BOARD_ROOT ${BOARD_ROOT_ABSOLUTE})

message(STATUS "CUSTOM_BOARD_ROOT absolute path: ${CUSTOM_BOARD_ROOT}")

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

mcux_set_variable(harpoon_root_path "../../harpoon-apps")
mcux_set_variable(harpoon_os_board_path "${harpoon_root_path}/common/freertos/boards/${board}")
mcux_set_variable(harpoon_app_path "${harpoon_root_path}/industrial")
mcux_set_variable(harpoon_app_board_path "${harpoon_app_path}/boards/${board}")
mcux_set_variable(harpoon_app_os_board_path "${harpoon_app_path}/freertos/boards/${board}")

project(industrial LANGUAGES C ASM PROJECT_BOARD_PORT_PATH ${harpoon_app_board_path})

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    SOURCES main.c
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${harpoon_app_board_path}
             ${harpoon_os_board_path}
             ${harpoon_root_path}/common
             ${harpoon_root_path}/common/boards/${board}
             ${harpoon_root_path}/common/freertos
             ${harpoon_app_path}/freertos
             ${harpoon_app_os_board_path}
             ${harpoon_app_path}/common
             ${harpoon_app_path}/common/boards/${board}
             ${harpoon_app_path}/common/boards/include
)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${harpoon_os_board_path}/mmu.c
            ${harpoon_os_board_path}/board.c
            ${harpoon_app_path}/common/boards/${board}/clock_config.c
            ${harpoon_app_path}/common/boards/${board}/hardware_ethernet.c
            ${harpoon_app_path}/common/boards/${board}/pin_mux.c
            ${harpoon_app_path}/common/industrial.c
            ${harpoon_app_path}/freertos/boards/${board}/stub.c
)

# Common FreeRTOS configurations
include(${SdkRootDirPath}/${harpoon_root_path}/common/freertos/common_freertos.cmake)
include(${SdkRootDirPath}/${harpoon_root_path}/common/freertos/core/armv8a/common_freertos_core_armv8a.cmake)
include(${SdkRootDirPath}/${harpoon_root_path}/common/libs/ctrl/lib_ctrl.cmake)
include(${SdkRootDirPath}/${harpoon_root_path}/common/libs/jailhouse/lib_jailhouse.cmake)
include(${SdkRootDirPath}/${harpoon_root_path}/common/libs/rpmsg/lib_rpmsg.cmake)

# Application-specific reconfig
include(${SdkRootDirPath}/${harpoon_app_os_board_path}/reconfig.cmake OPTIONAL)

# Abstraction Layer and Libraries
include(${SdkRootDirPath}/../../rtos-apps/mcux_rtos_apps.cmake)
include(${SdkRootDirPath}/../../rtos-abstraction-layer/freertos/mcux_rtos_abstraction_layer.cmake)

# Middlewares
# Include rpmsg-lite middleware manually
mcux_add_cmakelists(${SdkRootDirPath}/../../rpmsg-lite)

if(CONFIG_GENAVB_ENABLE)
    include(${SdkRootDirPath}/${harpoon_app_path}/freertos/genavb_sdk.cmake)
endif()

if(CONFIG_MOTOR_CONTROLLER)
    include(${SdkRootDirPath}/${harpoon_app_path}/freertos/motor_controller.cmake)
endif()

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin)
