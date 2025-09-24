# Description: lib providing helper functions for Generic software mailbox
include_guard(GLOBAL)
message("lib_gen_sw_mbox component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/gen_sw_mbox.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)
