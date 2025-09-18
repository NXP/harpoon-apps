mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${harpoon_app_path}/avb_tsn/tsn_app/motor
)

mcux_add_macro(CC "BUILD_MOTOR_CONTROLLER")
