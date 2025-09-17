#!/bin/sh

ZEPHYR_TOOLCHAIN_VARIANT=cross-compile
CROSS_COMPILE=$(find "$ARMGCC_DIR" -name "*-gcc" | sed -e 's/gcc$//')
export ZEPHYR_TOOLCHAIN_VARIANT
export CROSS_COMPILE

west build -p always ../../../ -b imx8mp_evk/mimx8ml8/a53/smp -d build_smp
