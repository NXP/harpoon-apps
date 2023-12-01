set(GenAVBPath "${ProjDirPath}/../../../../../../gen_avb_sdk")

# RTOS abstraction layer configuration
set(RTOS_ABSTRACTION_LAYER_DIR "${GenAVBPath}/freertos/freertos")
set(GenAVBBuildPath "${CMAKE_CURRENT_BINARY_DIR}/gen_avb_build")

set(FREERTOS_SDK ${SdkDirPath})
set(FREERTOS_DIR ${RtosPath})
set(FREERTOS_APPS "${ProjDirPath}/../../../../..")
set(TARGET "freertos_imx8mn_ca53")
set(CONFIG "endpoint_avb")

add_subdirectory(${GenAVBPath} "${GenAVBBuildPath}")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    "${AppPath}/common/audio_element_avtp_sink.c"
    "${AppPath}/common/audio_element_avtp_source.c"
    "${AppPath}/common/avb_config.c"
    "${AppPath}/common/boards/${BoardName}/avb_hardware.c"
    "${CommonPath}/boards/${BoardName}/genavb_sdk.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${GenAVBPath}/include
    ${GenAVBPath}/include/freertos
)

list(APPEND CMAKE_MODULE_PATH
    ${CommonPath}/libs/avb_tsn
)

add_library(avb-core-lib STATIC IMPORTED)

set_target_properties(avb-core-lib PROPERTIES IMPORTED_LOCATION ${GenAVBBuildPath}/libstack-core.a)

add_library(srp-lib STATIC IMPORTED)
set_target_properties(srp-lib PROPERTIES IMPORTED_LOCATION ${GenAVBBuildPath}/srp)

add_dependencies(${MCUX_SDK_PROJECT_NAME} avb-core-lib stack-freertos srp-lib)

include(lib_avb_tsn)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)
target_link_libraries(srp-lib INTERFACE stack-freertos)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE avb-core-lib)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE stack-freertos)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)
