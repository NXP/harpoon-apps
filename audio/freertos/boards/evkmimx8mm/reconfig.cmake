mcux_add_macro(
    CC " \
        ENET_ENHANCEDBUFFERDESCRIPTOR_MODE \
        -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
")

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${harpoon_app_path}/common/pipeline_config.c
)
