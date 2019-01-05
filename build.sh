#!/bin/bash

# First we create the binary image of the OS
# Our OS is written in NASM assembly code
nasm -f bin -o bootloader.bin bootloader.asm
nasm -f bin -o kernel.bin kernel.asm

# Create a floppy image that can be run using QEMU
dd conv=notrunc if=kernel.bin of=avos.flp seek=3
dd conv=notrunc if=bootloader.bin of=avos.flp

# Delete the binary file - we don't need it anymore
# If you feel like it, comment the line below and keep the binary file
#rm *.bin
