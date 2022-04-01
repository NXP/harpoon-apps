#!/bin/bash

function usage ()
{
	echo "harpoon_set_configuration.sh <audio | industrial | latency>"
}

function detect_machine ()
{
	if grep -q 'i.MX8MM' /sys/devices/soc0/soc_id; then
		SOC=imx8mm
		ENTRY=0x93c00000
	elif grep -q 'i.MX8MN' /sys/devices/soc0/soc_id; then
		SOC=imx8mm
		ENTRY=0x93c00000
	elif grep -q 'i.MX8MP' /sys/devices/soc0/soc_id; then
		SOC=imx8mp
		ENTRY=0xc0000000
	fi
}

CONF_FILE=/etc/harpoon/harpoon.conf

if [ ! $# -eq 1 ]; then
	usage
	exit 1
fi

detect_machine

if [ -z $SOC ]; then
	echo "Unexpected SOC: " $(cat /sys/devices/soc0/soc_id)
	exit 2
fi

if [ $1 == "audio" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-freertos-audio.cell
	INMATE_BIN=/usr/share/harpoon/inmates/audio.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=freertos
	EOF
elif [ $1 == "industrial" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-freertos-industrial.cell
	INMATE_BIN=/usr/share/harpoon/inmates/industrial.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=freertos
	EOF
elif [ $1 == "latency" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-freertos.cell
	INMATE_BIN=/usr/share/harpoon/inmates/rt_latency.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=freertos
	EOF
else
	usage
	exit 2
fi
