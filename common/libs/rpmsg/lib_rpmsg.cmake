# Description: lib providing helper functions for rpmsg-lite
include_guard(GLOBAL)
message("lib_rpmsg component is included.")

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}/../gen_sw_mbox/
)

target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PRIVATE AARCH64=1)

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/rpmsg.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)

include(lib_gen_sw_mbox)
