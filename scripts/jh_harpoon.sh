#!/bin/bash -e
# For now the script assumes:
#  - a single jailhouse cell is running/will run
#  - the cell runs one of the Harpoon images

function usage ()
{
    echo "usage: jh_harpoon.sh <start | stop>"
}

# Detect the machine we are running on.
detect_machine ()
{
    if grep -q 'FSL i\.MX8MM.*EVK.*board' /sys/devices/soc0/machine; then
        echo 'imx8mmevk'
    elif grep -q 'NXP i\.MX8MNano EVK board' /sys/devices/soc0/machine; then
        echo 'imx8mnevk'
    elif grep -q 'NXP i\.MX8MPlus EVK board' /sys/devices/soc0/machine; then
        echo 'imx8mpevk'
    elif grep -q 'NXP i\.MX93.*EVK.*board' /sys/devices/soc0/machine; then
        echo 'imx93evk'
    elif grep -q 'NXP i\.MX95 19X19 board' /sys/devices/soc0/machine; then
        echo 'imx95evk19'
    else
        echo 'Unknown'
    fi
}

function get_rpmsg_dev()
{
    case $MACHINE in
    'imx8mmevk'|'imx8mnevk')
        RPMSG_DEV=b8600000.rpmsg-ca53
        ;;
    'imx8mpevk')
        RPMSG_DEV=fe100000.rpmsg-ca53
        ;;
    'imx93evk')
        RPMSG_DEV=fe100000.rpmsg-ca55
        ;;
    'imx95evk19')
        RPMSG_DEV=c0100000.rpmsg-ca55
        ;;
    *)
        echo "Unsupported Machine: ${MACHINE}"
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
    if [ -e /sys/devices/system/cpu/cpu"$1"/power/pm_qos_resume_latency_us ]; then
        echo "1" > /sys/devices/system/cpu/cpu"$1"/power/pm_qos_resume_latency_us
    fi
}

# Check if a list of strings contains an element
# $1: list to parse
# $2: item to check
function listincludes()
{
    for word in $1; do
        [[ $word = "$2" ]] && return 0
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

    read -r -a avail_governors <  /sys/devices/system/cpu/cpufreq/policy0/scaling_available_governors

    if listincludes "${avail_governors[*]}" "performance" ; then
        echo "Setting Performance as CPU frequency scaling governor"
        echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
    elif listincludes "${avail_governors[*]}" "userspace" ; then
        read -r -a avail_frequencies <  /sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies
        max_freq=${avail_frequencies[-1]}
        echo "Setting Userspace as CPU frequency scaling governor at $max_freq"
        echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
        echo "$max_freq" > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed
    else
        echo "Error: can not set performance or userspace as CPU frequency scaling governor"
        exit 1
    fi
}


# $1: register address in hex
function get_reg_value()
{
    local reg_addr="$1"

    echo "0x$(/unit_tests/memtool "$reg_addr" 1 | grep -i "$reg_addr": | sed "s/$reg_addr: *//gi")"
}

function set_ddrc_configuration()
{
    local value=0, timing_orig=0, mstr=0, pwrctl=0, rfshctl=0, rfshtmg=0, new_timing=0

    if [ "$MACHINE" = "imx93evk" ]; then
        if [ -e /sys/devices/platform/imx93-lpm/auto_clk_gating ]; then
            echo "Disable auto clock gating"
            echo 0 > /sys/devices/platform/imx93-lpm/auto_clk_gating
        fi
    elif [ "$MACHINE" = "imx8mnevk" ] || [ "$MACHINE" = "imx8mmevk" ] || [ "$MACHINE" = "imx8mpevk" ]; then
        echo "Disable DDR Low-Power Mode"
        # Disable selfref_en and powerdown_en in PWRCTL
        pwrctl=$(get_reg_value "0x3D400030")
        value=$(printf '0x%X' $(( pwrctl & ~0x3 )))
        /unit_tests/memtool 0x3d400030="${value}"

        # Check if we are using LPDDR4
        mstr=$(get_reg_value "0x3D400000")
        if [ $(( mstr & 0x20 )) -eq "0" ]; then
            return;
        fi

        # If current configuration is using all bank refresh, set the minimum refresh rate to a per bank value
        # with the assumption of having 8 banks (common SDRAM configuration).
        rfshctl=$(get_reg_value "0x3D400050")
        if [ $(( rfshctl & 0x4 )) -eq "0" ]; then
            echo "DDRC: Enable per-bank LPDDR4 refresh"
            rfshtmg=$(get_reg_value "0x3D400064")
            value=$(printf '0x%X' $(( rfshtmg & ~0x3ff )))
            timing_orig=$(printf '0x%X' $(( rfshtmg & 0x3ff )))
            new_timing=$(printf '0x%X' $(( timing_orig / 8 )))
            value=$(printf '0x%X' $(( value | new_timing )))
            echo "DDRC: Change tRFC(min) from all bank value (${timing_orig}) to per bank value (${new_timing})"
            /unit_tests/memtool 0x3d400064="${value}"
            # Now enable per bank refresh
            rfshctl=$(get_reg_value "0x3D400050")
            value=$(printf '0x%X' $(( rfshctl | 0x4 )))
            /unit_tests/memtool 0x3d400050="${value}"
        fi
    fi
}

function disable_cpu_idle_all()
{
    if [ "$MACHINE" = "imx93evk" ]; then
        # Disable CPU idle for all cores
        disable_cpu_idle 0
        disable_cpu_idle 1
    elif [ "$MACHINE" = "imx8mnevk" ] || [ "$MACHINE" = "imx8mmevk" ] || [ "$MACHINE" = "imx8mpevk" ]; then
        # Disable CPU idle for all cores
        disable_cpu_idle 0
        disable_cpu_idle 1
        disable_cpu_idle 2
        disable_cpu_idle 3
    elif [ "$MACHINE" = "imx95evk19" ]; then
        # Disable CPU idle for all cores
        disable_cpu_idle 0
        disable_cpu_idle 1
        disable_cpu_idle 2
        disable_cpu_idle 3
        disable_cpu_idle 4
        disable_cpu_idle 5
    fi
}

function disable_rtc_device()
{
    if [ "$MACHINE" = "imx93evk" ]; then
        # Unbind the BBNSM RTC device
        BBNSM_RTC_DEV="44440000.bbnsm:rtc"
        if [ -L /sys/bus/platform/drivers/bbnsm_rtc/${BBNSM_RTC_DEV} ]; then
            echo "Unbind RTC device"
            echo "${BBNSM_RTC_DEV}" > /sys/bus/platform/drivers/bbnsm_rtc/unbind
        fi
    elif [ "$MACHINE" = "imx8mnevk" ] || [ "$MACHINE" = "imx8mmevk" ] || [ "$MACHINE" = "imx8mpevk" ]; then
        # Unbind the RTC device
        RTC_DEV="30370000.snvs:snvs-rtc-lp"
        if [ -L /sys/bus/platform/drivers/snvs_rtc/${RTC_DEV} ]; then
            echo "Unbind RTC device"
            echo "${RTC_DEV}" > /sys/bus/platform/drivers/snvs_rtc/unbind
        fi
    elif [ "$MACHINE" = "imx95evk19" ]; then
        # Unbind the SCMI RTC device
        SCMI_DEV="scmi_dev.11"
        if [ -L /sys/bus/scmi_protocol/drivers/scmi-imx-bbm/${SCMI_DEV} ]; then
            echo "Unbind RTC device"
            echo "${SCMI_DEV}" > /sys/bus/scmi_protocol/drivers/scmi-imx-bbm/unbind
        fi
    fi
}

function set_real_time_configuration()
{
    set_ddrc_configuration

    disable_cpu_idle_all

    disable_rtc_device

    set_cpu_freq_policy
}

function gpio_start ()
{
    if [[ "${INMATE_BIN}" =~ .*"industrial.bin" && "$MACHINE" = "imx93evk" ]]; then
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
    if [[ "${INMATE_BIN}" =~ .*"industrial.bin" && "$MACHINE" = "imx93evk" ]]; then
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

        if [[ -n "${RPMSG_DEV}" ]] && [[ -L "/sys/bus/platform/drivers/imx-rpmsg/${RPMSG_DEV}" ]]; then
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
        if [[ -n "${RPMSG_DEV}" ]]; then
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

# Detect the Machine we're running on.
MACHINE=$(detect_machine)

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

if [ "$1" == "start" ]; then
    start
elif [ "$1" == "stop" ]; then
    stop
else
    usage
    exit 1
fi
