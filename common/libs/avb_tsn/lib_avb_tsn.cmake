# Description: lib providing helper functions to link with genAVB/TSN stack
include_guard(GLOBAL)
message("lib_avb_tsn component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/clock_domain.c
    ${CMAKE_CURRENT_LIST_DIR}/crf_stream.c
    ${CMAKE_CURRENT_LIST_DIR}/genavb.c
    ${CMAKE_CURRENT_LIST_DIR}/genavb_sdk.c
    ${CMAKE_CURRENT_LIST_DIR}/log.c
    ${CMAKE_CURRENT_LIST_DIR}/media_clock.c
    ${CMAKE_CURRENT_LIST_DIR}/stats_task.c
    ${CMAKE_CURRENT_LIST_DIR}/system_config.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)

set(RTOS_ABSTRACTION_LAYER_TARGET ${MCUX_SDK_PROJECT_NAME})
include(${RTOS_ABSTRACTION_LAYER_DIR}/rtos_abstraction_layer.cmake)
