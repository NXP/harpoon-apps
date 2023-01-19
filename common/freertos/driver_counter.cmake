#Description: Harpoon FreeRTOS specific implentation; user_visible: True
include_guard(GLOBAL)
if (NOT ${MCUX_DEVICE} STREQUAL "MIMX9352_ca55")
message("counter driver component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/os/counter.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${SdkDirPath}/drivers/gpt
)
else()
# TODO: add imx93 counter driver support
message("counter driver component is not included!")
endif()
