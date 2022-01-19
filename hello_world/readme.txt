Overview
========
The Hello World project is a simple demonstration program that uses the
MCUXpresso SDK's UART driver in combination with an RTOS.
The purpose of this demo is to show how to use the guest RTOS along with
a SoC driver.

The example application creates one task called hello_task. This task prints
"Hello world." message via debug console utility and suspends itself.


Toolchain supported
===================
- GCC ARM 10.3-2021.07 for aarch64
- GCC ARM 10.2-2020.11 for aarch64

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMX8M Plus board
- Power supply
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect the power supply to the board
2.  Connect a USB cable between the host PC and the debug USB port on the target board.
3.  Open two serial terminals with the following settings (ttyUSB2 and ttyUSB3):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Create a jailhouse guest cell and download the program into the latter
5.  Start the jailhouse cell

Running the demo
================
Once the program is executed the serail terminal shows "Hello world" message.
