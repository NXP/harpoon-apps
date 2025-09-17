#Description: Harpoon FreeRTOS specific implentation; user_visible: True
include_guard(GLOBAL)
message("TPM counter driver component is included.")

mcux_add_source(
    SOURCES
    os/counter.c
    os/counter_tpm.c
)

mcux_add_include(
    INCLUDES
    drivers/tpm
)
