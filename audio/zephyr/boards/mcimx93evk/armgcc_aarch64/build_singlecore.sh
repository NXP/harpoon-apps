#!/bin/sh

ZEPHYR_TOOLCHAIN_VARIANT=cross-compile
CROSS_COMPILE=$(find "$ARMGCC_DIR" -name "*-gcc" | sed -e 's/gcc$//')
export ZEPHYR_TOOLCHAIN_VARIANT
export CROSS_COMPILE

west build -p always ../../../ -b imx93_evk/mimx9352/a55 -d build_singlecore
