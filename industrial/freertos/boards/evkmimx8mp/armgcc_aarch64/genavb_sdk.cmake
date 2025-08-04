set(GenAVBPath "${ProjDirPath}/../../../../../../gen_avb_sdk")

# RTOS abstraction layer configuration
set(RTOS_ABSTRACTION_LAYER_DIR "${ProjDirPath}/../../../../../../rtos-abstraction-layer/freertos")
set(GenAVBBuildPath "${CMAKE_CURRENT_BINARY_DIR}/gen_avb_build")

set(MCUX_SDK ${SdkDirPath})
set(RTOS_DIR ${RtosPath})
set(RTOS_APPS "${ProjDirPath}/../../../../..")
set(TARGET "freertos_imx8mp_ca53")
set(CONFIG "endpoint_tsn")

add_subdirectory(${GenAVBPath} "${GenAVBBuildPath}")

set(build_motor_controller 1) # motor_control built by default

# Overwrite unsupported configurations
add_compile_definitions(SERIAL_MODE=0)
add_compile_definitions(BUILD_IO_DEVICE=0) # io_device unsupported
add_compile_definitions(BUILD_MOTOR_CONTROLLER=${build_motor_controller})

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppPath}/avb_tsn/tsn_app/configs.c
    ${AppPath}/common/ethernet_avb_tsn.c
    ${CommonPath}/boards/${BoardName}/genavb_sdk.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${GenAVBPath}/include
    ${GenAVBPath}/include/rtos
)

if(build_motor_controller)
    include(${CMAKE_CURRENT_LIST_DIR}/motor_controller.cmake)
endif()

list(APPEND CMAKE_MODULE_PATH
    ${CommonPath}/libs/avb_tsn
)

add_library(avb-core-lib STATIC IMPORTED)

set_target_properties(avb-core-lib PROPERTIES IMPORTED_LOCATION ${GenAVBBuildPath}/libstack-core.a)

add_dependencies(${MCUX_SDK_PROJECT_NAME} avb-core-lib stack-rtos)

include(lib_avb_tsn)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE avb-core-lib)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE stack-rtos)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)
