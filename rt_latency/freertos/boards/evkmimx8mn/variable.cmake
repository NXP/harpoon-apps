mcux_set_variable(board evkmimx8mn)

if (NOT DEFINED device)
    mcux_set_variable(device MIMX8MN6)
endif()

include(${SdkRootDirPath}/devices/i.MX/i.MX8MN/${device}/variable.cmake)
