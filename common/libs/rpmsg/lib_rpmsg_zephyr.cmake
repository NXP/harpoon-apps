# config to select component, the format is CONFIG_USE_${component}
set(CONFIG_USE_middleware_zephyr-kernel true)
set(CONFIG_USE_middleware_multicore_rpmsg_lite_zephyr true)

include(lib_rpmsg)

include(middleware_multicore_rpmsg_lite_aarch64_zephyr)
include(middleware_multicore_rpmsg_lite_zephyr)

if(CONFIG_BOARD_MIMX8MM_EVK_A53)
  include(middleware_multicore_rpmsg_lite_MIMX8MM6)
elseif(CONFIG_BOARD_MIMX8MN_EVK_A53)
  include(middleware_multicore_rpmsg_lite_MIMX8MN6)
elseif(CONFIG_BOARD_MIMX8MP_EVK_A53)
  include(middleware_multicore_rpmsg_lite_MIMX8ML8)
elseif(CONFIG_BOARD_MIMX93_EVK_A55)
  include(middleware_multicore_rpmsg_lite_MIMX9352)
else()
  message(FATAL_ERROR "unsupported board")
endif()