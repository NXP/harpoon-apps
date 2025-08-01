# Description: lib providing helper functions to link with genAVB/TSN stack
include_guard(GLOBAL)
message("lib_avb_tsn component is included.")

if(${CONFIG} STREQUAL "endpoint_avb")
    include(${CMAKE_CURRENT_LIST_DIR}/aem-manager.cmake)
    add_definitions(-DCONFIG_GENAVB_USE_AVDECC=1)
endif()

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/genavb.c
    ${CMAKE_CURRENT_LIST_DIR}/genavb_sdk.c
    ${CMAKE_CURRENT_LIST_DIR}/log.c
    ${CMAKE_CURRENT_LIST_DIR}/system_config.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)

set(RTOS_ABSTRACTION_LAYER_TARGET ${MCUX_SDK_PROJECT_NAME})
include(${RTOS_ABSTRACTION_LAYER_DIR}/rtos_abstraction_layer.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/lib_avb_tsn_stats.cmake)
