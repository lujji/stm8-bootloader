# stm8-bootloader
A serial bootloader for low-density STM8S microcontrollers.

... work in progress ...

TL;DR

Short PD4 -> GND.
``` bash
$ make && make flash
$ cd app && make
$ python ../uploader/boot.py --p /dev/ttyUSB0 firmware.bin
```
