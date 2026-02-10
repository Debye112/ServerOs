#!/bin/sh
set -e

arm-none-eabi-gcc -c -O2 -ffreestanding -nostdlib kernel.c -o kernel.o
arm-none-eabi-as boot.S -o boot.o
arm-none-eabi-ld -T linker.ld boot.o kernel.o -o kernel.elf

qemu-system-arm -M versatilepb -m 128M -kernel kernel.elf -hda ~/qemu_disks/os_disk.qcow2 -nongraphic

