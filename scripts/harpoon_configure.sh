#!/bin/bash

# Runtime configuration script: should be called only from the harpoon control application.
# harpoon_configure.sh <usecase> <command>

# Detect the platform we are running on.
function detect_machine ()
{
	if grep -q 'i.MX8MM' /sys/devices/soc0/soc_id; then
		SOC=imx8mm
	elif grep -q 'i.MX8MN' /sys/devices/soc0/soc_id; then
		SOC=imx8mn
	elif grep -q 'i.MX8MP' /sys/devices/soc0/soc_id; then
		SOC=imx8mp
	elif grep -q 'i.MX93' /sys/devices/soc0/soc_id; then
		SOC=imx93
	fi
}

# $1: audio command
function audio_process()
{
	case $1 in
	'start')
		if [ "$SOC" == "imx93" ]; then
			# Make sure there is no previously launched gpioset daemon to avoid device busy errors
			killall -q gpioset

			# Set EXP_SEL GPIO as output low to configure WM8962 codec
			gpioset -z -c gpiochip5 4=0 > /dev/null 2>&1
		fi
		;;

	'start_audio_hat')
		if [ "$SOC" == "imx93" ]; then
			# Make sure there is no previously launched gpioset daemon to avoid device busy errors
			killall -q gpioset

			# Set EXP_SEL GPIO as output high to configure CS42448 codec
			gpioset -z -c gpiochip5 4=1 > /dev/null 2>&1
		fi
		;;

	'stop')
		if [ "$SOC" == "imx93" ]; then
			# Kill created gpioset daemon by start cmd
			killall -q gpioset || true
		fi
		;;
	esac
}

if [ ! $# -eq 2 ]; then
	echo "Wrong number of arguments"
	exit 1
fi

detect_machine

if [ "$1" == "audio" ]; then
	audio_process $2
fi