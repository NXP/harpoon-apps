#!/bin/bash -e
# For now the script assumes:
#  - a single jailhouse cell is running/will run
#  - the cell runs one of the Harpoon images

function usage ()
{
    echo "usage: jh_harpoon.sh <start | stop>"
}

# Detect the platform we are running on.
function detect_soc ()
{
    if grep -q 'i.MX8MM' /sys/devices/soc0/soc_id; then
        echo 'imx8mm'
    elif grep -q 'i.MX8MN' /sys/devices/soc0/soc_id; then
        echo 'imx8mn'
    elif grep -q 'i.MX8MP' /sys/devices/soc0/soc_id; then
        echo 'imx8mp'
    elif grep -q 'i.MX93' /sys/devices/soc0/soc_id; then
        echo 'imx93'
    else
        echo 'Unknown'
    fi
}

function get_rpmsg_dev()
{
    case $SOC in
    'imx8mm'|'imx8mn')
        RPMSG_DEV=b8600000.rpmsg-ca53
        ;;
    'imx8mp')
        RPMSG_DEV=fe100000.rpmsg-ca53
        ;;
    'imx93')
        RPMSG_DEV=fe100000.rpmsg-ca55
        ;;
    *)
        echo "Unsupported SoC"
        exit 1
    esac

    if [[ ! -L /sys/bus/platform/devices/${RPMSG_DEV} ]]; then
        unset RPMSG_DEV
        echo 'The RPMsg device does not exist!'
    fi
}

# $1: cpu core number
function disable_cpu_idle()
{
    # Disable CPU idle deep state transitions exceeding 1us
    if [ -e "/sys/devices/system/cpu/cpu"$1"/power/pm_qos_resume_latency_us" ]; then
        echo "1" > /sys/devices/system/cpu/cpu"$1"/power/pm_qos_resume_latency_us
    fi
}

# Check if a list of strings contains an element
# $1: list to parse
# $2: item to check
function listincludes()
{
    for word in $1; do
        [[ $word = $2 ]] && return 0
    done

    return 1
}

# Disable run-time CPU frequency changes:
# - Skip if CPU Freq is disabled
# - Otherwise, set performance or userspace governor with a fixed frequency
function set_cpu_freq_policy()
{
    if [ ! -d /sys/devices/system/cpu/cpufreq ] || \
       [ ! -f /sys/devices/system/cpu/cpufreq/policy0/scaling_available_governors ]; then
        echo "CPU Frequency Scaling is not available, skip configuration"
        return;
    fi

    avail_governors=$(cat /sys/devices/system/cpu/cpufreq/policy0/scaling_available_governors)

    if listincludes "$avail_governors" "performance" ; then
        echo "Setting Performance as CPU frequency scaling governor"
        echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
    elif listincludes "$avail_governors" "userspace" ; then
        avail_frequencies=($(cat /sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies))
        max_freq=${avail_frequencies[-1]}
        echo "Setting Userspace as CPU frequency scaling governor at $max_freq"
        echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
        echo "$max_freq" > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed
    else
        echo "Error: can not set performance or userspace as CPU frequency scaling governor"
        exit 1
    fi
}

function set_real_time_configuration()
{
    if [ "$SOC" = "imx93" ]; then
        if [ -e /sys/devices/platform/imx93-lpm/auto_clk_gating ]; then
            echo "Disable auto clock gating"
            echo 0 > /sys/devices/platform/imx93-lpm/auto_clk_gating
        fi

        # Disable CPU idle for all cores
        disable_cpu_idle 0
        disable_cpu_idle 1

        # Unbind the BBNSM RTC device
        BBNSM_RTC_DEV="44440000.bbnsm:rtc"
        if [ -L /sys/bus/platform/drivers/bbnsm_rtc/${BBNSM_RTC_DEV} ]; then
            echo "Unbind RTC device"
            echo "${BBNSM_RTC_DEV}" > /sys/bus/platform/drivers/bbnsm_rtc/unbind
        fi
    fi

    set_cpu_freq_policy
}

function gpio_start ()
{
    if [[ "${INMATE_BIN}" =~ .*"industrial.bin" && "$SOC" = "imx93" ]]; then
        # Set ADP5585 EXP_SEL GPIO as output low
        echo 'gpioset -z -c gpiochip5 4=0'
        gpioset -z -c gpiochip5 4=0

        # Set ADP5585 CAN_STBY GPIO as output low
        echo 'gpioset -z -c gpiochip5 5=0'
        gpioset -z -c gpiochip5 5=0
    fi
}

function gpio_stop ()
{
    # Kill all gpioset commands and restore GPIOs state
    if [[ "${INMATE_BIN}" =~ .*"industrial.bin" && "$SOC" = "imx93" ]]; then
        echo 'killall "gpioset"'
        killall "gpioset"

        # Restore input direction for ADP5585 EXP_SEL GPIO
        echo 'gpioget -l -c gpiochip5 4'
        gpioget -l -c gpiochip5 4
        echo 'gpioget -l -c gpiochip5 5'
        gpioget -l -c gpiochip5 5
    fi
}

function start ()
{
    set_real_time_configuration

    if [[ ! "${INMATE_BIN}" =~ .*"virtio_net.bin" && ! "${INMATE_BIN}" =~ .*"hello_world.bin" ]]; then
        get_rpmsg_dev

        if [[ ! -z "${RPMSG_DEV}" ]] && [[ -L "/sys/bus/platform/drivers/imx-rpmsg/${RPMSG_DEV}" ]]; then
            echo 'unbind the rpmsg-ca53 from imx_rpmsg driver'
            echo "${RPMSG_DEV}" > /sys/bus/platform/drivers/imx-rpmsg/unbind
        fi
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

    if [[ ! "${INMATE_BIN}" =~ .*"virtio_net.bin" && ! "${INMATE_BIN}" =~ .*"hello_world.bin" ]]; then
        if [[ ! -z "${RPMSG_DEV}" ]]; then
            # delay here to ensure the slave side ready to kick
            sleep 0.5
            echo 're-bind the rpmsg-ca53 to imx_rpmsg driver'
            echo "${RPMSG_DEV}" > /sys/bus/platform/drivers/imx-rpmsg/bind
        fi
    fi

    if [[ "${INMATE_BIN}" =~ .*"virtio_net.bin" ]]; then
        echo 'modprobe -r virtio_net'
        modprobe -r virtio_net
        echo 'modprobe -r virtio_mmio'
        modprobe -r virtio_mmio
        echo 'modprobe virtio_mmio'
        modprobe virtio_mmio
        echo 'modprobe virtio_net'
        modprobe virtio_net
    fi

    gpio_start
}

function stop ()
{
    gpio_stop

    if [[ ! "${INMATE_BIN}" =~ .*"virtio_net.bin" && ! "${INMATE_BIN}" =~ .*"hello_world.bin" ]]; then
        get_rpmsg_dev
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

# Detect the SoC we're running on.
SOC=$(detect_soc)

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
