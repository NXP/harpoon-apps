#!/bin/bash

function usage ()
{
	echo "harpoon_set_configuration.sh <freertos | zephyr> <audio | industrial | latency>"
}

function detect_machine ()
{
	if grep -q 'i.MX8MM' /sys/devices/soc0/soc_id; then
		SOC=imx8mm
		ENTRY=0x93c00000
	elif grep -q 'i.MX8MN' /sys/devices/soc0/soc_id; then
		SOC=imx8mn
		ENTRY=0x93c00000
	elif grep -q 'i.MX8MP' /sys/devices/soc0/soc_id; then
		SOC=imx8mp
		ENTRY=0xc0000000
	fi
}

CONF_FILE=/etc/harpoon/harpoon.conf

if [ ! $# -eq 2 ]; then
	usage
	exit 1
fi

detect_machine

RTOS="$1"
if [[ "$RTOS" != "freertos" && "$RTOS" != "zephyr" ]]; then
	usage
	exit 2
fi

if [ -z $SOC ]; then
	echo "Unexpected SOC: " "$(cat /sys/devices/soc0/soc_id)"
	exit 3
fi

if [ "$2" == "audio" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-${RTOS}-audio.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/audio.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "industrial" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-${RTOS}-industrial.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/industrial.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "latency" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-${RTOS}.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/rt_latency.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
else
	usage
	exit 4
fi
