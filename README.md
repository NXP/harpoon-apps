# Harpoon: Reference Applications

[![Version](https://img.shields.io/github/v/release/NXP/harpoon-apps)](https://github.com/NXP/harpoon-apps/releases/latest)
[![Contributors](https://img.shields.io/github/contributors/NXP/harpoon-apps)](https://github.com/NXP/harpoon-apps/graphs/contributors)

Harpoon is a software platform providing an RTOS and application specific drivers, running under the Jailhouse hypervisor and side-by-side with a full Linux distribution on the Cortex-A cores of NXP MPU’s (i.MX 8M Mini, i.MX 8M Nano, i.MX 8M Plus, i.MX 93 and i.MX 95).

Harpoon offers an environment for developing real-time demanding applications, leveraging the higher performance of the Cortex-A cores (ARMv8-A) compared to the Cortex-M cores (ARMv7-M/ARMv8-M) traditionally used for RTOS based applications.

The reference applications embedded in this repository run on several Jailhouse guest RTOS (Zephyr and FreeRTOS) leveraging the existing NXP RTOS development environment – MCUXpresso SDK.

To ease the integration of Zephyr, mcux-sdk and other repositories, we leverage the [Zephyr west tool](https://docs.zephyrproject.org/latest/guides/west/index.html) which helps managing multiple repositories and allows users to maintain their own ```west.yml``` for their project.

# Overview

This project provides the fundamental support for RTOS applications running in an hypervisor environment:
* Hardware resource partitioning using Jailhouse hypervisor, providing an inter-cell IPC communication
* Arm® CMSIS-Core device drivers and header files for 64-bit Cortex-A53/A55 cores
* RTOS kernel integration (Zephyr and FreeRTOS)
* Open-source peripheral drivers leveraging the MCUXpresso SDK ( [mcux-sdk project](https://github.com/nxp-mcuxpresso/mcux-sdk) )
* Integration in Linux Yocto BSP as well as Real-time Edge Software Yocto through the [meta-nxp-harpoon](https://github.com/NXP/meta-nxp-harpoon) layer (integration with systemd)

This project is the main repository to build the RTOS application for ARMv8-A cores, it contains the [west.yml](https://github.com/NXP/harpoon-apps/blob/main/west.yml) which keeps the description and revision for other projects in the overall Harpoon delivery. Currently available software components are shown in below figure.

![sdk_graph](docs/sdk_graph.png)

# Getting Started

A cross compiler is required to build Cortex-A applications ; this project is compatible with the ARM GCC toolchain that you may download and install:

```bash
wget https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-elf.tar.xz
tar -C /opt/ -xvf arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-elf.tar.xz     # (sudo permissions might be required)
```

You need to have the following tools to clone and build Harpoon applications:
```bash
# Update environment
sudo apt update

# git
sudo apt install git

# python3
sudo apt install python3-pip

# west
pip3 install west

# Common tools for compilation
sudo apt install cmake # minimum required version "3.20.0"

# Zephyr Compilation
sudo apt install ninja-build
sudo apt install python3-pyelftools
```

# Cloning this repository

Execute below commands to replicate the whole Harpoon delivery at revision ```${revision}``` and place it in a west workspace named ```hww```.
```bash
export revision=harpoon_3.3.0
west init -m https://github.com/NXP/harpoon-apps --mr ${revision} hww
cd hww
west update
```
Replace ```${revision}``` with any Harpoon release you wish to use. This can also be ```main``` if you want the latest state, or any commit SHA.

# Repository structure

The aim of this repository is to provide a comprehensive set of reference applications.
It provides a `west` manifest to fetch not only Zephyr, but also FreeRTOS as well as the MCUXpresso SDK (including drivers and libraries) and multiple middleware stacks:

```bash
.
.
├── harpoon-apps
│   ├── common                              <-- source code common to all applications
│   │   ├── freertos
│   │   │   ├── boards
│   │   │   │   ├── evkmimx8mm
│   │   │   │   │   ├── armgcc_aarch64
│   │   │   │   │   │   ├── flags.cmake
│   │   │   │   │   │   ├── MIMX8MM6xxxxx_ca53_ddr_ram.ld <-- linker script
│   │   │   │   │   │   └── MIMX8MM6xxxxx_ca53_ram.ld
│   │   │   │   │   ├── board.c
│   │   │   │   │   ├── board.h             <-- board-specific board configuration for all applications
│   │   │   │   │   ├── clock_config.h
│   │   │   │   │   └── mmu.c               <-- board-specific MMU regions for all applications
│   │   │   │    ...
│   │   │   ├── common_freertos.cmake       <-- hardware-agnostic source code for FreeRTOS
│   │   │   ├── core                        <-- includes os-specific header files for the os APIs
│   │   │   │   └── armv8a
│   │   │   │       ├── common_freertos_core_armv8a.cmake
│   │   │   │       └── startup.S           <-- ARMv8-A startup code
│   │   │   ├── FreeRTOSConfig.h
│   │   │   ├── FreeRTOS_helper.c
│   │   │   ├── FreeRTOS_tick_config.c
│   │   │   ├── idle.{c,h}                  <-- implementation of idle task
│   │   │   ├── irq.{c,h}                   <-- contains interrupt handler
│   │   │   ├── mmu.h                       <-- glue with the SDK MMU driver
│   │   │   ├── os                          <-- includes os-specific header files for the os APIs
│   │   │   │   ├── assert.h
│   │   │   │   ├── counter.{c,h}
│   │   │   │   ├── semaphore.h
│   │   │   │   ├── stdio.h
│   │   │   │   └── unistd.h
│   │   │   └── os.h
│   │   ├── libs
│   │   │   └── stats
│   │   │       ├── lib_stats.cmake
│   │   │       ├── stats.c
│   │   │       └── stats.h
│   │   ├── os                              <-- includes standard/posix header files os APIs
│   │   │   ├── assert.h
│   │   │   ├── cache.h
│   │   │   ├── counter.h
│   │   │   ├── semaphore.h
│   │   │   ├── stdio.h
│   │   │   └── unistd.h
│   │   └── zephyr
│   │       ├── os
│   │       │   ├── assert.h
│   │       │   ├── counter.h
│   │       │   ├── semaphore.h
│   │       │   ├── stdio.h
│   │       │   └── unistd.h
│   │       └── os.h
│   ├── harpoon_drivers_test                <-- top directory of the application
│   │   ├── freertos
│   │   │   ├── boards                      <-- board-specific source code used for FreeRTOS
│   │   │   │   ├── evkmimx8mm
│   │   │   │   │   ├── app_board.h         <-- optional header file for application-specific board configuration definitions
│   │   │   │   │   ├── app_mmu.h           <-- optional header file for application-specific MMU regions mapping for this board
│   │   │   │   │   └── armgcc_aarch64      <-- entry point to build this application for FreeRTOS/evkmimx8mm
│   │   │   │   │       ├── build_ddr_debug.sh
│   │   │   │   │       ├── build_ddr_release.sh
│   │   │   │   │       ├── clean.sh
│   │   │   │   │       └── CMakeLists.txt
│   │   │   │    ...
│   │   │   └── main.c                      <-- main entry point for FreeRTOS
│   │   ├── include
│   │   │   └── i2c_test.h
│   │   └── src
│   │       └── i2c_test.c                  <-- hardware/os-independent source code for the application
|   ├── audio                <-- RTOS audio application
│   ├── rt_latency           <-- RTOS rt latency measurement application
│   ├── industrial           <-- RTOS industrial application
│   ├── virtio_net           <-- RTOS virtio application
│   ├── README.md
│   └── west.yml
├── FreeRTOS-Kernel                         <-- RTOS Kernel Git tree
├── gen_avb_sdk                             <-- GenAVB/TSN stack
├── heterogeneous-multicore                 <--- Heterogeneous Multicore Framework
├── mcux-sdk                                <-- MCUXpresso Git tree
│   ├── ...
│   ├── components
│   ├── devices
│   │   ├── <SoC>
│   │   │   ├── drivers                     <-- SoC-dependent drivers and hardware definitions
│   │    ...
│   ├── docs
│   ├── drivers                             <-- IP-dependent drivers
│   ├── middleware
│   ├── tools
│   └── utilities
└── middleware                              <-- multicore and rpmsg-lite middleware
└── rtos-abstraction-layer                  <-- RTOS Abstraction Layer
└── zephyr                                  <-- Imported Zephyr module
```

# Reference Applications

The first application that you may attempt to build and execute is the `rt_latency` application.

## Compile

### FreeRTOS
If starting from a fresh console, the cross-compiler variable needs to be set:

```bash
export ARMGCC_DIR=/opt/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf
```

Then move to the right path, depending on the board and the RTOS that you are looking for. The example below builds the application within FreeRTOS and for the i.MX 8M Plus EVK board:

```bash
cd harpoon-apps/rt_latency/freertos/boards/evkmimx8mp/armgcc_aarch64/
./build_ddr_release.sh
```

The resulting binary is located under the `ddr_release/` directory and is called `rt_latency.bin`. This is the binary blob that _jailhouse_ loads into the inmate cell before starting it.

### Zephyr
If starting from a fresh console, the cross-compiler and zephyr variables need to be set:

```bash
export ARMGCC_DIR=/opt/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf
export Zephyr_DIR=/path/to/hww/zephyr
```

Then move to the right path, depending on the board and the RTOS that you are looking for. The example below builds the Zephyr `rt_latency` application for the i.MX 8M Plus EVK board:

```bash
cd harpoon-apps/rt_latency/zephyr/boards/evkmimx8mp/armgcc_aarch64/
./clean.sh
./build_singlecore.sh
```
Please refer to https://docs.zephyrproject.org/latest/develop/west/install.html for more details.

### Using Build Helper Script
"build_apps.sh" is a build helper script. It can be used to build one or more applications by specifying application name, board and/or target OS. The following is the script's usage information:
```bash
./build_apps.sh [clean]                                 - build or clean all applications
./build_apps.sh [clean] [os] [board-list] [app-list]    - build or clean specified applications
                                                        - os: freertos or zephyr (both if not specified)
                                                        - board-list: one or more of: evkmimx8mm evkmimx8mn evkmimx8mp mcimx93evk imx95lpd5evk19 (all if not specified)
                                                        - app-list: one or more of: audio harpoon_drivers_test hello_world industrial rt_latency virtio_net (all if not specified)
```
Before executing the script, as mentioned above, the enviroment variables "ARMGCC_DIR" and "Zephyr_DIR" (if building Zephyr applications) need to be set.

Then execute the script, for example:

Building all applications in harpoon_apps:
```bash
./build_apps.sh
```
Building all FreeRTOS audio and rt_latency applications:
```bash
./build_apps.sh freertos audio rt_latency
```
Cleaning all previously built applications in harpoon_apps:
```bash
./build_apps.sh clean
```
Cleaning all hello_world applications previously built in harpoon_apps:
```bash
./build_apps.sh hello_world clean
```
All resulting binaries can be found in "deploy/images" root directory of harpoon_apps, and clean command will delete related binaries in "deploy/images" directory.

## Running the reference applications

Jailhouse, running in the Linux root cell, provides the necessary tools to create, load and execute the reference applications built within this repository ; this example gives the commands for a inmate cell for i.MX 8MP EVK, replace "xxx" with "freertos" for FreeRTOS or "zephyr" for Zephyr:

```bash
# RPMSG device on i.MX 8M Plus EVK for example
RPMSG_DEV=fe100000.rpmsg-ca53
# unbind the rpmsg device from imx_rpmsg driver
echo "${RPMSG_DEV}" > /sys/bus/platform/drivers/imx-rpmsg/unbind

modprobe jailhouse
jailhouse enable /usr/share/jailhouse/cells/imx8mp.cell
jailhouse cell create /usr/share/jailhouse/cells/imx8mp-harpoon-xxx.cell
jailhouse cell load xxx /usr/share/harpoon/inmates/xxx/rt_latency.bin --address 0xc0000000
jailhouse cell start xxx

# bind the rpmsg device to imx_rpmsg driver
echo "${RPMSG_DEV}" > /sys/bus/platform/drivers/imx-rpmsg/bind

harpoon_ctrl latency -r 1   # start rt_latency test case 1

jailhouse cell shutdown xxx
jailhouse cell destroy xxx
jailhouse disable
modprobe -r jailhouse
```

Alternatively, a systemd unit file is provided to start the reference applications. This unit file runs a scripts that uses configuration file `/etc/harpoon/harpoon.conf` to figure out the different jailhouse parameters (application name, cell names, load address, ...).
Preconfigured configurations can be generated with script `harpoon_set_configuration.sh`.

Example to run the FreeRTOS `rt_latency` application:

```bash
harpoon_set_configuration.sh freertos latency # this may be run only once; replace freertos by zephyr for Zephyr applications
systemctl start harpoon
```

To be able to visualize the guest OS console, the UART4 tty shall be opened ; e.g.:

```bash
screen /dev/ttyUSB3 115200
```

Please refer to the [Harpoon User's Guide](https://www.nxp.com/docs/en/user-guide/Harpoon_USERS_GUIDE.pdf) for more details.
