# config to select component, the format is CONFIG_USE_${component}
set(CONFIG_USE_middleware_zephyr-kernel true)
set(CONFIG_USE_middleware_multicore_rpmsg_lite_zephyr true)

include(lib_rpmsg)

include(middleware_multicore_rpmsg_lite_aarch64_zephyr)
include(middleware_multicore_rpmsg_lite_zephyr)

if(CONFIG_BOARD_IMX8MM_EVK)
  include(middleware_multicore_rpmsg_lite_MIMX8MM6)
elseif(CONFIG_BOARD_IMX8MN_EVK)
  include(middleware_multicore_rpmsg_lite_MIMX8MN6)
elseif(CONFIG_BOARD_IMX8MP_EVK)
  include(middleware_multicore_rpmsg_lite_MIMX8ML8)
elseif(CONFIG_BOARD_IMX93_EVK)
  include(middleware_multicore_rpmsg_lite_MIMX9352)
elseif(CONFIG_BOARD_IMX95_EVK)
  include(middleware_multicore_rpmsg_lite_MIMX9596)
elseif(CONFIG_BOARD_IMX95_EVK_15X15)
  include(middleware_multicore_rpmsg_lite_MIMX9596)
else()
  message(FATAL_ERROR "unsupported board")
endif()
