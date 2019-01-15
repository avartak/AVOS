#!/bin/bash

nasm -f bin   -o bootloader.bin boot/src/bootloader.asm
nasm -f bin   -o kload16.bin    boot/src/kload16.asm 
nasm -f bin   -o kload32.bin    boot/src/kload32.asm 

nasm -f elf32 -o start.o        kernel/src/start.asm

gcc --target=i386-jos-elf -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -I /Users/avartak/AVOS/AVOS -c csupport/src/memory.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -I /Users/avartak/AVOS/AVOS -c kernel/src/kernel.c

i386-elf-ld -m elf_i386 -T link.ld  -o kernel.bin  start.o kernel.o memory.o

# Create a floppy image that can be run using QEMU
dd conv=notrunc if=kernel.bin     of=avos.flp seek=40
dd conv=notrunc if=kload32.bin    of=avos.flp seek=24
dd conv=notrunc if=kload16.bin    of=avos.flp seek=8
dd conv=notrunc if=bootloader.bin of=avos.flp

rm *.bin *.o
