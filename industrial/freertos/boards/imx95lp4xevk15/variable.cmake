mcux_set_variable(board imx95lp4xevk15)

if (NOT DEFINED device)
    mcux_set_variable(device MIMX9596)
endif()

include(${SdkRootDirPath}/devices/i.MX/i.MX95/${device}/variable.cmake)
