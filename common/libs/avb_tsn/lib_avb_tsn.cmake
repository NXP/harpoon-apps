# Description: lib providing helper functions to link with genAVB/TSN stack
include_guard(GLOBAL)
message("lib_avb_tsn component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/board.c
    ${CMAKE_CURRENT_LIST_DIR}/genavb.c
    ${CMAKE_CURRENT_LIST_DIR}/log.c
    ${CMAKE_CURRENT_LIST_DIR}/stats_task.c
    ${CMAKE_CURRENT_LIST_DIR}/system_config.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)
