CMAKE_MINIMUM_REQUIRED (VERSION 3.12.0)

project(harpoon_ctrl)

SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
SET(CommonPath "${ProjDirPath}/../common")

set(MCUX_SDK_PROJECT_NAME harpoon_ctrl)

set(CMAKE_MODULE_PATH
    ${CommonPath}/libs/ctrl
)

add_executable(${MCUX_SDK_PROJECT_NAME}
   audio_pipeline.c
   common.c
   industrial.c
   main.c
   rpmsg.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CommonPath}
    ${ProjDirPath}
)

include(lib_ctrl)
