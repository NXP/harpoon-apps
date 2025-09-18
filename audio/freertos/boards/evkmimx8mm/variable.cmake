mcux_set_variable(board evkmimx8mm)

if (NOT DEFINED device)
    mcux_set_variable(device MIMX8MM6)
endif()

include(${SdkRootDirPath}/devices/i.MX/i.MX8MM/${device}/variable.cmake)
