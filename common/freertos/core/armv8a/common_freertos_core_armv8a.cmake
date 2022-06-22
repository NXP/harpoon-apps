#Description: Harpoon FreeRTOS specific implentation; user_visible: True
include_guard(GLOBAL)
message("common_freertos_core_armv8a component is included.")


target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/startup.S
    ${CMAKE_CURRENT_LIST_DIR}/exception.c
)
