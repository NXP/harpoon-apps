#!/bin/sh

app_root_path="../../../"
board=imx95lpd5evk19
build_target=ddr_release
cpu=ca55
output_dir=${build_target}

west sdk_build -p always ${app_root_path} -b ${board} --config ${build_target} -Dcore_id=${cpu} -d ${output_dir}
