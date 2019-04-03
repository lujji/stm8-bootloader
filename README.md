# stm8-bootloader
Serial bootloader for STM8S and STM8L microcontrollers. A detailed write-up on this bootloader is posted [here](https://lujji.github.io/blog/serial-bootloader-for-stm8).

## Features

* **small** - fits in 547 bytes (SDCC v3.6.8 #9951)
* **fast** - uploads 4k binary in 1 second @115200bps
* **configurable** - can be adjusted for parts with different flash block size

## Configuration

The default configuration targets low-density devices (STM8S003). To compile for a different target `MCU` and `FAMILY` makefile variables are adjusted accordingly.

Bootloader configuration is located in `config.h`.
* **BLOCK_SIZE** - flash block size according to device datasheet. This should be set to 64 for low-density devices or 128 for devices with >8k flash.
* **BOOT_ADDR** - application boot address.
* **BOOT_PIN** - entry jumper. This is set to PD3 by default.
* **RELOCATE_IVT** - when set to 1 (default) the interrupt vectors are relocated. When set to 0 the bootloader will overwrite it's own interrupt vector table with the application's IVT, thus eliminating additional CPU overhead during interrupts. Write-protection cannot be used in this case and resulting binary is slightly larger.

### Changing boot address
Boot address must be a multiple of BLOCK_SIZE. Address is set in 2 places:
 * config.h
 * init.s

Main application is compiled with `--code-loc <address>` option. When RELOCATE_IVT is set to 0, 0x80 must be subtracted from application address and isr29 must be implemented: `void dummy_isr() __interrupt(29) __naked { ; }`.

## Build instructions
Build and flash the bootloader:

``` bash
$ make && make flash
```

Enable write-protection (UBC) on pages 0-9 _(TODO: must be adjusted for STML)_:

``` bash
$ make opt-set
```

## Uploading the firmware

There is a demo application inside `app` directory which toggles PD4 via interrupts. To upload the application short PD3 to ground, power-cycle the MCU and run the uploader utility. DTR pin on UART-USB converter can be connected to RESET pin on STM8 for automatic reset.

``` bash
$ cd app && make
$ python ../uploader/boot.py -p /dev/ttyUSB0 firmware.bin
```
