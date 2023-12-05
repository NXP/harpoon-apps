# config to select component, the format is CONFIG_USE_${component}

if(${MCUX_DEVICE} STREQUAL "MIMX8MM6_ca53")
set(CONFIG_USE_middleware_freertos-kernel_MIMX8MM6 true)
elseif(${MCUX_DEVICE} STREQUAL "MIMX8MN6_ca53")
set(CONFIG_USE_middleware_freertos-kernel_MIMX8MN6 true)
elseif(${MCUX_DEVICE} STREQUAL "MIMX8ML8_ca53")
set(CONFIG_USE_middleware_freertos-kernel_MIMX8ML8 true)
elseif(${MCUX_DEVICE} STREQUAL "MIMX9352_ca55")
set(CONFIG_USE_middleware_freertos-kernel_MIMX9352 true)
endif()
set(CONFIG_USE_middleware_multicore_rpmsg_lite_freertos true)

include(lib_rpmsg)

include(middleware_multicore_rpmsg_lite_aarch64_freertos)
include(middleware_multicore_rpmsg_lite_freertos)

if(${MCUX_DEVICE} STREQUAL "MIMX8MM6_ca53")
include(middleware_multicore_rpmsg_lite_MIMX8MM6)
elseif(${MCUX_DEVICE} STREQUAL "MIMX8MN6_ca53")
include(middleware_multicore_rpmsg_lite_MIMX8MN6)
elseif(${MCUX_DEVICE} STREQUAL "MIMX8ML8_ca53")
include(middleware_multicore_rpmsg_lite_MIMX8ML8)
elseif(${MCUX_DEVICE} STREQUAL "MIMX9352_ca55")
include(middleware_multicore_rpmsg_lite_MIMX9352)
endif()
