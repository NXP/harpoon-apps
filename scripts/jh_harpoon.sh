#!/bin/bash -e
# For now the script assumes:
#  - a single jailhouse cell is running/will run
#  - the cell runs one of the Harpoon images



function usage ()
{
    echo "usage: jh_harpoon.sh <start | stop>"
}

function start ()
{
    echo 'modprobe -r virtio_rpmsg_bus'
    modprobe -r virtio_rpmsg_bus

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

    echo 'modprobe virtio_rpmsg_bus'
    modprobe virtio_rpmsg_bus
}

function stop ()
{
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
