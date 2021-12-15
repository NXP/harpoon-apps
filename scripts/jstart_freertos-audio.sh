#!/bin/bash

ROOT_CELL=imx8mm.cell
INMATE_CELL=imx8mm-freertos-audio.cell
INMATE_CELL_NAME=freertos
#BINS_PATH=/usr/share/harpoon/inmates
BINS_PATH=/opt

INMATE_BIN=${BINS_PATH}/audio.bin
INMATE_ENTRY_ADDRESS=0xc0000000

echo 'modprobe jailhouse'
modprobe jailhouse

echo 'Enabling jailhouse root cell'
jailhouse enable /usr/share/jailhouse/cells/${ROOT_CELL}

echo 'Creating jailhouse inmate cell'
jailhouse cell create /usr/share/jailhouse/cells/${INMATE_CELL}

echo 'Loading jailhouse inmate cell'
jailhouse cell load ${INMATE_CELL_NAME} ${LK_BIN} -a ${INMATE_ENTRY_ADDRESS}

echo 'Starting inmate cell'
jailhouse cell start ${INMATE_CELL_NAME}
