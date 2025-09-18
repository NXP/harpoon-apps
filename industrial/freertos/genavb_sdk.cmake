include(${SdkRootDirPath}/../../gen_avb_sdk/mcux_genavb.cmake)

mcux_set_variable(CONFIG "endpoint_tsn")
mcux_set_variable(GenAVBPath "../../gen_avb_sdk")
mcux_set_variable(RTOS_ABSTRACTION_LAYER_DIR "${SdkRootDirPath}/../../rtos-abstraction-layer/freertos")
include(${SdkRootDirPath}/${harpoon_root_path}/common/libs/avb_tsn/lib_avb_tsn.cmake)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES
            ${harpoon_app_path}/avb_tsn/tsn_app/configs.c
            ${harpoon_app_path}/common/ethernet_avb_tsn.c
            ${harpoon_root_path}/common/boards/${board}/genavb_sdk.c
)
