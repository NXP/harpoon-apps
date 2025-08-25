mcux_set_variable(board mcimx93evk)

if (NOT DEFINED device)
    mcux_set_variable(device MIMX9352)
endif()

include(${SdkRootDirPath}/devices/i.MX/i.MX93/${device}/variable.cmake)
