#!/bin/bash

function usage ()
{
	echo "harpoon_set_configuration.sh <freertos | zephyr> <audio | audio_smp | avb | hello | industrial | latency | virtio_net>"
	echo "virtio_net: this application is only available for FreeRTOS on evkimx8mm/p and evkimx93"
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
	elif grep -q 'i.MX93' /sys/devices/soc0/soc_id; then
		SOC=imx93
		ENTRY=0xd0000000
	elif grep -q 'i.MX95' /sys/devices/soc0/soc_id; then
		SOC=imx95
		ENTRY=0xf0000000
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

if [ "$2" == "virtio_net" ]; then
	if [[ "$SOC" == "imx8mn" || "$RTOS" != "freertos"  ]]; then
		usage
		exit 7
	fi
fi

if [ "$2" == "audio" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-harpoon-${RTOS}-audio.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/audio.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "audio_smp" ]; then
	if [[ "$RTOS" != "zephyr" || "$SOC" == "imx93" ]]; then
		echo "Unsupported RTOS"
		exit 4
	fi
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-harpoon-${RTOS}-audio.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/audio_smp.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "avb" ]; then
	if [[ "$RTOS" == "zephyr" && "$SOC" != "imx93" ]]; then
		INMATE_BIN="/usr/share/harpoon/inmates/${RTOS}/audio_smp.bin"
	else
		INMATE_BIN="/usr/share/harpoon/inmates/${RTOS}/audio.bin"
	fi
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-harpoon-${RTOS}-avb.cell
	INMATE_BIN=${INMATE_BIN}
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "hello" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-harpoon-${RTOS}.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/hello_world.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "industrial" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-harpoon-${RTOS}-industrial.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/industrial.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "latency" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-harpoon-${RTOS}.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/rt_latency.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "virtio_net" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-harpoon-${RTOS}-virtio.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/virtio_net.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
else
	usage
	exit 6
fi
