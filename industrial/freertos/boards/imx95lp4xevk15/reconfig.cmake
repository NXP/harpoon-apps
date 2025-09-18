mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES
            ${harpoon_app_path}/common/boards/${board}/hardware_flexcan.c
            ${harpoon_app_path}/common/ethernet.c
            ${harpoon_app_path}/common/can.c
)

mcux_add_source(
    SOURCES
            clock_setup.c
            ethernet_setup.c
            pinctrl.c
)
mcux_add_include(
    INCLUDES .
)

include(${SdkRootDirPath}/${harpoon_root_path}/common/freertos/driver_counter_tpm.cmake)
