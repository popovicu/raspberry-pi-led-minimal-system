# Raspberri Pi LED minimal system

**This repo is meant to accompany the article at http://popovicu.com/posts/10-mb-raspberry-pi-mainline-linux-image-for-embedded**

The final image (uncompressed) is 10.4 MB. Kernel image is ~6 MB.

End structure:
* bcm2708-rpi-zero.dtb
* bootcode.bin
* cmdline.txt
* config.txt
* fixup.dat
* initrd.gz
* overlays
* start.elf
* zImage

Sample UART output from booting Raspberry Pi Zero with this set up is in `output.txt`.

## Compiling the init process

```
arm-linux-gnueabi-gcc -static -o init init.c
```

## Connecting to the UART console on Raspberry Pi

```
sudo screen /dev/ttyUSB0 115200
```

## Building the kernel

```
ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- make bcm2835_defconfig
```

```
ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- make -j16
```