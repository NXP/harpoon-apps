#!/bin/bash

# usage:
# build or clean all applications: ./build_apps [clean]
# build or clean specified applications: ./build_apps [clean]  [os] [board-list] [app-list]

valid_apps=("audio" "harpoon_drivers_test" "hello_world" "industrial" "rt_latency" "virtio_net")

valid_boards=("evkmimx8mm" "evkmimx8mn" "evkmimx8mp" "mcimx93evk")

valid_oss=("freertos" "zephyr")

freertos_cmd="./build_ddr_release.sh"
zephyr_cmd="./build_singlecore.sh"
zephyr_smp_cmd="./build_smp.sh"
clean_cmd="clean.sh"
is_clean="no"

cmds=("${freertos_cmd}" "${zephyr_cmd}" "${zephyr_smp_cmd}")

# all binary images will be copied to this directory
images_dir_name="deploy/images"
deploy_dir_name="deploy"

function print_usage() {
    echo "$0 [clean]					- build or clean all applications"
    echo "$0 [clean] [os] [board-list] [app-list]	- build or clean specified applications"
    echo "							- os: specify freertos or zephyr or both if no specified."
    echo "							- board-list: specify one or some or all boards if no specified: ${valid_boards[*]}"
    echo "							- app-list: specify one or some or all applications if no specified: ${valid_apps[*]}"
}

apps_list=()
boards_list=()
oss_list=()

# $1: array name to lookup
# $2: element to search
function array_contains_elem() {
    lookup_array="$1[@]"

    # loop over all array elements
    for each_elem in "${!lookup_array}"; do
        if [[ $each_elem == "$2" ]]; then
            return 0
        fi
    done

    return 1
}

function check_app() {
    is_app="no"
    if array_contains_elem "valid_apps" "$1"; then
        is_app="yes"
        apps_list+=("$1")
    fi
}

function check_board() {
    is_board="no"
    if array_contains_elem "valid_boards" "$1"; then
        is_board="yes"
        boards_list+=("$1")
    fi
}

function check_os() {
    is_os="no"
    if array_contains_elem "valid_oss" "$1"; then
        is_os="yes"
        oss_list+=("$1")
    fi
}

function del_empty_dir() {
    if [ -d "$1" ]; then
        if [ "$(ls -A "$1"|wc -w)" == "0" ]; then
            rm -rf "$1"
        fi
    fi
}

for each_parm in "$@"
do
    check_app "${each_parm}"
    if [[ ${is_app} == "no" ]]; then
        check_board "${each_parm}"
        if [[ ${is_board} == "no" ]]; then
            check_os "${each_parm}"
            if [[ ${is_os} == "no" ]]; then
                if [[ "${each_parm}" == "clean" ]]; then
                    is_clean="yes"
                else
                    echo "Invalid parameter ${each_parm}"
                    print_usage
                    exit 1
                fi
            fi
        fi
    fi
done

if [ ${#apps_list[@]} -eq 0 ]; then
    apps_list=("${valid_apps[@]}")
fi
if [ ${#boards_list[@]} -eq 0 ]; then
    boards_list=("${valid_boards[@]}")
fi
if [ ${#oss_list[@]} -eq 0 ]; then
    oss_list=("${valid_oss[@]}")
fi

harpoon_apps_dir=$(cd "$(dirname "$0")" || exit; pwd)

for each_app in "${apps_list[@]}"; do
    for each_os in "${oss_list[@]}"; do
        for each_board in "${boards_list[@]}"; do
            build_dir="${harpoon_apps_dir}/${each_app}/${each_os}/boards/${each_board}/armgcc_aarch64"
            if [ -d "${build_dir}" ]; then
                cd "${build_dir}" || exit
                if [ ${is_clean} == "yes" ]; then # clean
                    if [ -f "${clean_cmd}" ]; then
                        if ! source ${clean_cmd}; then
                            echo "clean ${each_os} ${each_app} application on ${each_board} failed"
                            exit 1
                        fi
                        # delete binary image
                        bin_file="${harpoon_apps_dir}/${images_dir_name}/${each_board}/${each_os}/${each_app}.bin"
                        if [ -f "${bin_file}" ]; then
                            rm -f "${bin_file}"
                        fi
                        smp_bin_file="${harpoon_apps_dir}/${images_dir_name}/${each_board}/${each_os}/${each_app}_smp.bin"
                        if [ -f "${smp_bin_file}" ]; then
                            rm -f "${smp_bin_file}"
                        fi
                        del_empty_dir "${harpoon_apps_dir}/${images_dir_name}/${each_board}/${each_os}"
                        del_empty_dir "${harpoon_apps_dir}/${images_dir_name}/${each_board}"
                        del_empty_dir "${harpoon_apps_dir}/${images_dir_name}"
                        del_empty_dir "${harpoon_apps_dir}/${deploy_dir_name}"
                    fi
                else # build
                    for cmd in "${cmds[@]}"; do
                        if [ -f "${cmd}" ]; then
                            echo "building ${each_os} ${each_app} application on ${each_board} ..."
                            if ! source "${cmd}"; then
                                echo "build ${each_os} ${each_app} application on ${each_board} failed"
                                exit 1
                            fi
                            # copy binary image
                            output_dir="${harpoon_apps_dir}/${images_dir_name}/${each_board}/${each_os}"
                            if [ ! -d "${output_dir}" ]; then
                                mkdir -p "${output_dir}"
                            fi
                            if [[ "${cmd}" =~ "smp" ]]; then
                                find . -name "${each_app}"_smp.bin -exec cp {} "${output_dir}" \;
                            else
                                find . -name "${each_app}".bin -exec cp {} "${output_dir}" \;
                            fi
                        fi
                    done
                fi
                # return back
                cd "${harpoon_apps_dir}" || exit
            fi
        done
    done
done
