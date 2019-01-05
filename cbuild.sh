#!/bin/bash

nasm -f bin   -o bootloader.bin bootloader.asm
nasm -f elf32 -o start.o        start.asm

gcc  -m32        -ffreestanding -o kmain.o    -c kmain.c
ld   -m elf_i386 -T linker.ld   -o kernel.bin start.o kmain.o

# Create a floppy image that can be run using QEMU
dd conv=notrunc if=kernel.bin of=avos.flp seek=3
dd conv=notrunc if=bootloader.bin of=avos.flp

rm *.bin *.o
