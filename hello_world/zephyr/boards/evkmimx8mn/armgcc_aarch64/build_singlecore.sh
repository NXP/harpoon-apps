#!/bin/sh
CMSIS_PATH=$(realpath ../../../../../../modules/hal/cmsis)
HAL_NXP_PATH=$(realpath ../../../../../../modules/hal/nxp)

ZEPHYR_TOOLCHAIN_VARIANT=cross-compile
CROSS_COMPILE=$(find "$ARMGCC_DIR" -name "*-gcc" | sed -e 's/gcc$//')
export ZEPHYR_TOOLCHAIN_VARIANT
export CROSS_COMPILE

cmake -B build_singlecore -GNinja -DZEPHYR_MODULES="$CMSIS_PATH;$HAL_NXP_PATH" -DBOARD=mimx8mn_evk_a53 ../../../
ninja -C build_singlecore
