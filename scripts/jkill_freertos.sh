#!/bin/bash

INMATE_CELL_NAME=freertos

echo 'Shutdown inmate cell'
jailhouse cell shutdown ${INMATE_CELL_NAME}

echo 'Destroy inmate cell'
jailhouse cell destroy ${INMATE_CELL_NAME}

echo 'Disabling Jailhouse root cell'
jailhouse disable

echo 'modprobe -r jailhouse'
modprobe -r jailhouse
