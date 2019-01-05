#!/bin/bash

nasm -f bin   -o bootloader.bin boot/src/bootloader.asm
nasm -f elf32 -o start.o        kernel/src/start.asm

gcc --target=i386-jos-elf -ffreestanding -I /Users/avartak/AVOS/AVOS -c kernel/src/*.c
gcc --target=i386-jos-elf -ffreestanding -I /Users/avartak/AVOS/AVOS -c drivers/src/memory.c drivers/src/*.c

i386-elf-ld -m elf_i386 -T linker.ld -o kernel.bin start.o kernel.o memory.o console.o

# Create a floppy image that can be run using QEMU
dd conv=notrunc if=kernel.bin of=avos.flp seek=3
dd conv=notrunc if=bootloader.bin of=avos.flp

rm *.bin *.o
