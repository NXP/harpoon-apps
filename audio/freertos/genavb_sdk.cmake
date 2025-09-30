mcux_set_variable(RTOS_ABSTRACTION_LAYER_DIR "${SdkRootDirPath}/../../rtos-abstraction-layer/freertos")
mcux_set_variable(GenAVBPath "${SdkRootDirPath}/../../gen_avb_sdk")
mcux_set_variable(CONFIG "endpoint_avb")

# Disable "array-bounds" that leads to an error when using "DNDEBUG", due to a skipped assert in fsl_enet.c in ENET_SetTsISRHandler.
set_source_files_properties(${SdkRootDirPath}/drivers/enet/fsl_enet.c PROPERTIES COMPILE_FLAGS "-Wno-array-bounds")

include(${GenAVBPath}/mcux_genavb.cmake)
include(${SdkRootDirPath}/${harpoon_root_path}/common/libs/avb_tsn/lib_avb_tsn.cmake)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES
            ${harpoon_app_path}/common/avb_config.c
            ${harpoon_app_path}/common/boards/${board}/avb_hardware.c
            ${harpoon_root_path}/common/boards/${board}/genavb_sdk.c
)
