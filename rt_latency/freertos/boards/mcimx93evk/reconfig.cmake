mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${harpoon_root_path}/common/boards/${board}/clock_init.c
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${harpoon_root_path}/common/boards/${board}
)

mcux_add_source(
    SOURCES clock_config.c
)

mcux_add_include(
    INCLUDES .
)

include(${SdkRootDirPath}/${harpoon_root_path}/common/freertos/driver_counter_tpm.cmake)
