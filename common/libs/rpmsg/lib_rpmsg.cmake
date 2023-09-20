# Description: lib providing helper functions for rpmsg-lite
include_guard(GLOBAL)
message("lib_rpmsg component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/rpmsg.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)

include(lib_gen_sw_mbox)
