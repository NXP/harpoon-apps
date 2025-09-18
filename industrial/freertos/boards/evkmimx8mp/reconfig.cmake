mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES
            ${harpoon_app_path}/common/boards/${board}/hardware_flexcan.c
            ${harpoon_app_path}/common/ethernet.c
            ${harpoon_app_path}/common/can.c
)

include(${SdkRootDirPath}/${harpoon_root_path}/common/freertos/driver_counter_gpt.cmake)
