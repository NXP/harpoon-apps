target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppPath}/avb_tsn/tsn_app/motor
)

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppPath}/avb_tsn/tsn_app/motor/controller.c
    ${AppPath}/avb_tsn/tsn_app/motor/control_strategies.c
    ${AppPath}/avb_tsn/tsn_app/motor/current_control.c
    ${AppPath}/avb_tsn/tsn_app/motor/motor_params.c
    ${AppPath}/avb_tsn/tsn_app/motor/scenarios.c
    ${AppPath}/avb_tsn/tsn_app/motor/traj_planner.c
)