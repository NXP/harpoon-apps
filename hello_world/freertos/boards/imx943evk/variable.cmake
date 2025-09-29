mcux_set_variable(board imx943evk)

if (NOT DEFINED device)
    mcux_set_variable(device MIMX94398)
endif()

include(${SdkRootDirPath}/devices/i.MX/i.MX943/${device}/variable.cmake)
