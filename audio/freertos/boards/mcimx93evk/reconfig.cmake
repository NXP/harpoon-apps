mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${harpoon_root_path}/common/boards/${board}/clock_init.c
            ${harpoon_app_path}/common/pipeline_config_single_sai.c
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${harpoon_root_path}/common/boards/${board}
)
