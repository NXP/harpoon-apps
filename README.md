# Harpoon Demo Applications

Harpoon is a Base Enablement SW platform, providing an RTOS and application specific drivers running in Linux and Jailhouse hypervisor environment, on the Cortex-A cores of NXP MPU’s (i.MX 8M Mini, i.MX 8M Nano, i.MX 8M Plus, ...).

Harpoon offers customers an environment for developping real-time demanding applications leveraging the higher performance of the Cortex-A cores (ARMv8) compared to the Cortex-M cores (ARMv7) traditionnally used for RTOS based applications.

The test applications embedded in this repository may run on several jailhouse guest RTOS (e.g.: Zephyr, FreeRTOS) leveraging the existing NXP RTOS development environment – MCUXpresso.

# Getting Started

```txt
TODO: insert link to user guide
```

A cross compiler is required to build Cortex-A applications ; this project is compatible with the ARM GCC toolchain that you may download and install:

```
wget https://developer.arm.com/-/media/Files/downloads/gnu-a/10.2-2020.11/binrel/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf.tar.xz
tar -C /opt/ -xvf gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf.tar.xz     # (sudo permissions might be required)
```

# Cloning this repository

```txt
# create a Harpoon west workspace:
mkdir hww
cd hww/
# setup up workspace using west commands:
west init -m ssh://git@bitbucket.sw.nxp.com/HRPN/harpoon-apps
west update
```

# Repository structure

The aim of this repository is to provide a comprehensive set of demo applications.
It provides a `west` manifest to fetch not only Zephyr, but also FreeRTOS as well as the MCUXpresso SDK (including drivers and libraries):

```txt
.
.
├── harpoon-apps
│   ├── common                              <-- source code common to all applications
│   │   ├── freertos
│   │   │   │   └── evkmimx8mp
│   │   │   │       ├── aarch64gcc
│   │   │   │       │   ├── MIMX8ML8xxxxx_ca53_ddr_ram.ld
│   │   │   │       │   └── startup_MIMX8ML8_ca53.S
│   │   │   │       ├── board.c
│   │   │   │       ├── board.h
│   │   │   │       └── clock_config.h
│   │   │   ├── common_freertos.cmake       <-- hardware-agnostic source code for FreeRTOS
│   │   │   ├── FreeRTOS_assert.c
│   │   │   ├── FreeRTOSConfig.h
│   │   │   ├── FreeRTOS_tick_config.c
│   │   │   ├── stdio.c                     <-- local implementation of the os APIs
│   │   │   ├── mmu.h
│   │   │   ├── os                          <-- includes os-specific header files for the os APIs
│   │   │   │   └── stdio.h
│   │   │   └── os.h
│   │   ├── libs
│   │   │   └── stats
│   │   │       ├── lib_stats.cmake
│   │   │       ├── stats.c
│   │   │       └── stats.h
│   │   ├── os                              <-- includes standard/posix header files os APIs
│   │   │   └── stdio.h
│   │   └── zephyr
│   │       ├── os
│   │       │   └── stdio.h
│   │       └── os.h
│   ├── hello_world                         <-- top-directory of the application
│   │   ├── freertos
│   │   │   ├── boards                      <-- board-specific source code used for FreeRTOS
│   │   │   │   └── evkmimx8mp
│   │   │   │       └── aarch64gcc          <-- entry point to build this application for FreeRTOS/evkmimx8mp
│   │   │   │           └── build_ddr_release.sh
│   │   │   └── main.c                      <-- main entry point for FreeRTOS
│   │   ├── include
│   │   │   └── hello_world.h
│   │   ├── readme.txt
│   │   ├── src
│   │   │   └── hello_world.c               <-- hardware/os-independant source code for the application
│   │   └── zephyr
│   │       └── main.c                      <-- main entry point for Zephyr
│   └── manifest-tmp
├── freertos                                <-- FreeRTOS-Kernel Git submodule
├── mcux-sdk                                <-- MCUXpresso Git submodule
│   ├── ...
│   ├── components
│   ├── devices
│   │   ├── <SoC>
│   │   │   ├── drivers                     <-- SoC-dependant drivers and hardware definitions
│   │    ...
│   ├── docs
│   ├── drivers                             <-- IP-dependant drivers
│   ├── middleware
│   ├── tools
│   └── utilities
└── zephyr                                  <-- Imported Zephyr module
```

# Demo Applications

The very first application that you may attempt to build and execute is the `hello_world` demo.

## Compile

### FreeRTOS

If starting from a fresh console, the cross-compiler variable needs to be set:

```
export ARMGCC_DIR=/opt/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf
```

Then move to the right path, depending on the board and the rtos that you are looking for. The example below builds the application within FreeRTOS and for the i.MX 8M Plus EVK board:

```
cd demo_apps/hello_world/freertos/boards/evkmimx8mp/aarch64gcc/
./build_ddr_release.sh
```

The resulting binary is located under the `ddr_release/` directory and is called `hello_world.bin`. This is the binary blob that _jailhouse_ loads into the guest cell before starting it.

### Zephyr

Since Zephyr is installed using `west`, it is recommended to export Zephyr before building an application:

```
west zephyr-export
west build -p auto -b mimx8mp_evk_a53_1core harpoon-apps/rt_latency/zephyr/testcase1/
```

## Running the tests

Jailhouse running in the Linux host cell provides the necessary tools to create, load and execute the demo applications built within this repository ; this examples gives the commands for a FreeRTOS guest cell for i.MX 8MP EVK:

```
jailhouse cell create /usr/share/jailhouse/cells/imx8mp-freertos.cell
jailhouse cell load freertos /opt/freertos_hello.bin --address 0xc0000000
jailhouse cell start freertos
```

To be able to interact with the guest OS console, the UART4 tty shall be opened ; e.g.:

```
screen /dev/ttyUSB3 115200
```
