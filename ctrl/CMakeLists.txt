CMAKE_MINIMUM_REQUIRED (VERSION 3.12.0)

project(harpoon_ctrl)

SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
SET(CommonPath "${ProjDirPath}/../common")

if("${MBOX_TRANSPORT_RPMSG}" STREQUAL "on")
    add_compile_definitions(MBOX_TRANSPORT_RPMSG)
    set(MCUX_SDK_PROJECT_NAME harpoon_ctrl_rpmsg)
else()
    set(MCUX_SDK_PROJECT_NAME harpoon_ctrl)
endif()

set(CMAKE_MODULE_PATH
    ${CommonPath}/libs/mailbox
    ${CommonPath}/libs/ctrl
)

add_executable(${MCUX_SDK_PROJECT_NAME}
   audio_pipeline.c
   common.c
   industrial.c
   ivshmem.c
   main.c
   rpmsg.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CommonPath}
    ${ProjDirPath}
)

include(lib_mailbox)
include(lib_ctrl)
