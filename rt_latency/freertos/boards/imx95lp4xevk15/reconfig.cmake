mcux_add_source(
    SOURCES clock_config.c
)

mcux_add_include(
    INCLUDES .
)

include(${SdkRootDirPath}/${harpoon_root_path}/common/freertos/driver_counter_tpm.cmake)
