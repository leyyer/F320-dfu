F320-dfu
========

C8051F320 DFU boot loader

The project use F320's usb function, emulating atmel atmega16u4's dfu mode.
use `flip` on window, or `dfu-programmer` to control firmware upgrading.

The user firmware start at 0x1000, erase flash only after 0x1000 blocks.
8051 IVT relocated to 0x1000 too.

Build the dfu.hex use SDCC. I only use sdcc toolchain to 51 MCUs.
