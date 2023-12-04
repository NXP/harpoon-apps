set(GenAVBPath "${ProjDirPath}/../../../../../../gen_avb_sdk")

# RTOS abstraction layer configuration
set(RTOS_ABSTRACTION_LAYER_DIR "${GenAVBPath}/rtos/freertos")
set(GenAVBBuildPath "${CMAKE_CURRENT_BINARY_DIR}/gen_avb_build")

set(FREERTOS_SDK ${SdkDirPath})
set(FREERTOS_DIR ${RtosPath})
set(FREERTOS_APPS "${ProjDirPath}/../../../../..")
set(TARGET "freertos_imx93_ca55")
set(CONFIG "endpoint_tsn")

add_subdirectory(${GenAVBPath} "${GenAVBBuildPath}")

# Overwrite unsupported configurations
add_compile_definitions(SERIAL_MODE=0)
add_compile_definitions(BUILD_MOTOR=0)

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppPath}/avb_tsn/tsn_app/alarm_task.c
    ${AppPath}/avb_tsn/tsn_app/configs.c
    ${AppPath}/avb_tsn/tsn_app/cyclic_task.c
    ${AppPath}/avb_tsn/tsn_app/tsn_task.c
    ${AppPath}/avb_tsn/tsn_app/tsn_tasks_config.c
    ${AppPath}/common/ethernet_avb_tsn.c
    ${CommonPath}/boards/${BoardName}/genavb_sdk.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${GenAVBPath}/include
    ${GenAVBPath}/include/rtos
)

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
