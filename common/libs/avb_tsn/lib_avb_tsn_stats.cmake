# Description: lib providing helper functions to link with genAVB/TSN stack
include_guard(GLOBAL)
message("lib_avb_tsn_stats component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/stats_task.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)
