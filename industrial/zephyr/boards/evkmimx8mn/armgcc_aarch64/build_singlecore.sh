#!/bin/sh

ZEPHYR_TOOLCHAIN_VARIANT=cross-compile
CROSS_COMPILE=$(find "$ARMGCC_DIR" -name "*-gcc" | sed -e 's/gcc$//')
export ZEPHYR_TOOLCHAIN_VARIANT
export CROSS_COMPILE

west build -p always ../../../ -b imx8mn_evk/mimx8mn6/a53 -d build_singlecore
