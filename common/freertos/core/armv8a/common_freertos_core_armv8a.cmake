#Description: Harpoon FreeRTOS specific implentation; user_visible: True
include_guard(GLOBAL)
message("common_freertos_core_armv8a component is included.")

mcux_add_source(
    SOURCES
    startup.S
    exception.c
    system.c
)
