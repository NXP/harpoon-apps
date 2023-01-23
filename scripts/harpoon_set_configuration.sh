#!/bin/bash

function usage ()
{
	echo "harpoon_set_configuration.sh <freertos | zephyr> <audio | audio_smp | avb | hello | industrial | latency> [rpmsg]"
	echo "rpmsg: this option is only available for freertos on evkimx8mm"
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
	fi
}

CONF_FILE=/etc/harpoon/harpoon.conf

RPMSG=""
if [ $# -eq 3 ]; then
	RPMSG="_$3"
	if [[ "${RPMSG}" != "_rpmsg" ]]; then
		usage
		exit 5
	fi
elif [ ! $# -eq 2 ]; then
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

if [[ "$RPMSG" == "_rpmsg" ]]; then
	if [[ "$SOC" != "imx8mm" || "$RTOS" != "freertos"  ]]; then
		usage
		exit 7
	fi
fi

if [ "$2" == "audio" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-${RTOS}-audio.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/audio${RPMSG}.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "audio_smp" ]; then
	if [[ "$RTOS" != "zephyr" ]]; then
		echo "Unsupported RTOS"
		exit 4
	fi
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-${RTOS}-audio.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/audio_smp.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "avb" ]; then
	if [[ "$RTOS" != "freertos" ]]; then
		echo "Unsupported RTOS"
		exit 4
	fi
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-${RTOS}-avb.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/audio.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "hello" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-${RTOS}.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/hello_world.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "industrial" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-${RTOS}-industrial.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/industrial${RPMSG}.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
elif [ "$2" == "latency" ]; then
	cat <<-EOF > "$CONF_FILE"
	ROOT_CELL=/usr/share/jailhouse/cells/${SOC}.cell
	INMATE_CELL=/usr/share/jailhouse/cells/${SOC}-${RTOS}.cell
	INMATE_BIN=/usr/share/harpoon/inmates/${RTOS}/rt_latency${RPMSG}.bin
	INMATE_ENTRY_ADDRESS=$ENTRY
	INMATE_NAME=${RTOS}
	EOF
else
	usage
	exit 6
fi
