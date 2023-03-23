#!/bin/bash -e
# For now the script assumes:
#  - a single jailhouse cell is running/will run
#  - the cell runs one of the Harpoon images


HC_RPMSG_SHORT_CUT=/usr/local/bin/harpoon_ctrl

function usage ()
{
    echo "usage: jh_harpoon.sh <start | stop>"
}

function start ()
{
    if [[ "${INMATE_NAME}" == "freertos" ]] && [[ ! "${INMATE_BIN}" =~ .*"hello_world.bin" ]]; then
        echo 'modprobe -r virtio_rpmsg_bus'
        modprobe -r virtio_rpmsg_bus

        echo 'Use RPMSG-based Linux control application'
        mkdir -p /usr/local/bin
        ln -sf /usr/bin/harpoon_ctrl_rpmsg "${HC_RPMSG_SHORT_CUT}"
    else
        rm -f "${HC_RPMSG_SHORT_CUT}"
    fi

    echo 'modprobe jailhouse'
    modprobe jailhouse

    echo 'Enabling jailhouse root cell'
    jailhouse enable "${ROOT_CELL}"

    echo 'Creating jailhouse inmate cell'
    jailhouse cell create "${INMATE_CELL}"

    echo 'Loading jailhouse inmate cell'
    jailhouse cell load "${INMATE_NAME}" "${INMATE_BIN}" -a "${INMATE_ENTRY_ADDRESS}"

    echo 'Starting inmate cell'
    jailhouse cell start "${INMATE_NAME}"

    if [[ "${INMATE_NAME}"  == "freertos" ]] && [[ ! "${INMATE_BIN}" =~ .*"hello_world.bin" ]]; then
        echo 'modprobe virtio_rpmsg_bus'
        modprobe virtio_rpmsg_bus
    fi

    if [[ "${INMATE_BIN}" =~ .*"virtio_net.bin" ]]; then
        echo 'modprobe -r virtio_net'
        modprobe -r virtio_net
        echo 'modprobe -r virtio_mmio'
        modprobe -r virtio_mmio
        sleep 5
        echo 'modprobe virtio_mmio'
        modprobe virtio_mmio
        echo 'modprobe virtio_net'
        modprobe virtio_net
    fi
}

function stop ()
{
    if [[ "${INMATE_NAME}" == "freertos" ]]; then
        echo 'Restore default (ivshmem) Linux control application'
        rm -f "${HC_RPMSG_SHORT_CUT}"
    fi

    if [[ "${INMATE_BIN}" =~ .*"virtio_net.bin" ]]; then
        echo 'modprobe -r virtio_net'
        modprobe -r virtio_net
        echo 'modprobe -r virtio_mmio'
        modprobe -r virtio_mmio
    fi

    echo 'Shutdown inmate cell'
    jailhouse cell shutdown 1

    echo 'Destroy inmate cell'
    jailhouse cell destroy 1

    echo 'Disabling Jailhouse root cell'
    jailhouse disable

    echo 'modprobe -r jailhouse'
    modprobe -r jailhouse
}


if [ ! $# -eq 1 ]; then
    usage
    exit 1
fi

CONF_FILE=/etc/harpoon/harpoon.conf

if [ ! -e "$CONF_FILE" ]; then
    >&2 echo "Missing configuration file."
    exit 2
fi

# Read jailhouse parameters from configuration file
source "$CONF_FILE"

if [ -z "$ROOT_CELL" ] || [ -z "$INMATE_CELL" ] || \
   [ -z "$INMATE_BIN" ] || [ -z "$INMATE_NAME" ] || \
   [ -z "$INMATE_ENTRY_ADDRESS" ]; then
    >&2 echo "Incomplete configuration file."
   exit 3
fi

if [ ! -e "$ROOT_CELL" ] || [ ! -e "$INMATE_CELL" ] || \
   [ ! -e "$INMATE_BIN" ]; then
    >&2 echo "Missing cell binary/file."
    exit 4
fi

if [ $1 == "start" ]; then
    start
elif [ $1 == "stop" ]; then
    stop
else
    usage
    exit 1
fi
