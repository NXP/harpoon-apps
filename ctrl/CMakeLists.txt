CMAKE_MINIMUM_REQUIRED (VERSION 3.1.1)

project(harpoon_ctrl)

SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
SET(CommonPath "${ProjDirPath}/../common")

set(MCUX_SDK_PROJECT_NAME harpoon_ctrl)

set(CMAKE_MODULE_PATH
    ${CommonPath}/libs/mailbox
    ${CommonPath}/libs/ctrl
)

add_executable(harpoon_ctrl
   audio_pipeline.c
   common.c
   ivshmem.c
   main.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CommonPath}
)

include(lib_mailbox)
include(lib_ctrl)
