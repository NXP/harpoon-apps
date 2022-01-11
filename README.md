# Harpoon Reference Applications

Harpoon is a Base Enablement SW platform, providing an RTOS and application specific drivers running in Linux and Jailhouse hypervisor environment, on the Cortex-A cores of NXP MPU’s (i.MX 8M Mini, i.MX 8M Nano, i.MX 8M Plus, ...).

Harpoon offers customers an environment for developing real-time demanding applications leveraging the higher performance of the Cortex-A cores (ARMv8-A) compared to the Cortex-M cores (ARMv7-M) traditionally used for RTOS based applications.

The reference applications embedded in this repository may run on several jailhouse guest RTOS (e.g.: FreeRTOS for now, and Zephyr soon) leveraging the existing NXP RTOS development environment – MCUXpresso.

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
# setup the workspace using west commands:
west init -m ssh://git@bitbucket.sw.nxp.com/HRPN/harpoon-apps
west update
```

# Repository structure

The aim of this repository is to provide a comprehensive set of reference applications.
It provides a `west` manifest to fetch FreeRTOS as well as the MCUXpresso SDK (including drivers and libraries):

```txt
.
.
├── harpoon-apps
│   ├── common                              <-- source code common to all applications
│   │   ├── cpu.h
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
│   │   │   ├── counter.c
│   │   │   ├── FreeRTOSConfig.h
│   │   │   ├── FreeRTOS_helper.c
│   │   │   ├── FreeRTOS_tick_config.c
│   │   │   ├── irq.{c,h}                   <-- contains interrupt handler
│   │   │   ├── mmu.h                       <-- glue with the SDK MMU driver
│   │   │   ├── os                          <-- includes os-specific header files for the os APIs
│   │   │   │   ├── assert.h
│   │   │   │   ├── counter.h
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
│   │   │   ├── counter.h
│   │   │   ├── semaphore.h
│   │   │   ├── stdio.h
│   │   │   └── unistd.h
│   │   └── <other RTOS>
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
│   ├── README.md
│   └── west.yml
├── FreeRTOS-Kernel                         <-- RTOS Kernel Git tree
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
└── ...
```

# Reference Applications

The first application that you may attempt to build and execute is the `rt_latency` application.

## Compile

### FreeRTOS

If starting from a fresh console, the cross-compiler variable needs to be set:

```
export ARMGCC_DIR=/opt/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf
```

Then move to the right path, depending on the board and the RTOS that you are looking for. The example below builds the application within FreeRTOS and for the i.MX 8M Plus EVK board:

```
cd harpoon-apps/rt_latency/freertos/boards/evkmimx8mp/armgcc_aarch64/
./build_ddr_release.sh
```

The resulting binary is located under the `ddr_release/` directory and is called `rt_latency.bin`. This is the binary blob that _jailhouse_ loads into the inmate cell before starting it.

## Running the reference applications

Jailhouse, running in the Linux root cell, provides the necessary tools to create, load and execute the reference applications built within this repository ; this example gives the commands for a FreeRTOS inmate cell for i.MX 8MP EVK:

```
modprobe jailhouse
jailhouse enable /usr/share/jailhouse/cells/imx8mp.cell
jailhouse cell create /usr/share/jailhouse/cells/imx8mp-freertos.cell
jailhouse cell load freertos /usr/share/harpoon/inmates/rt_latency.bin --address 0xc0000000
jailhouse cell start freertos

/usr/share/harpoon/harpoon_ctrl latency -r 1   # start rt_latency test case 1

jailhouse cell shutdown freertos
jailhouse cell destroy freertos
jailhouse disable
modprobe -r jailhouse
```

To be able to visualize the guest OS console, the UART4 tty shall be opened ; e.g.:

```
screen /dev/ttyUSB3 115200
```
