#!/bin/bash

export TARGET=i686-elf
export INCDIR="/Users/avartak/AVOS/AVOS"

export AS=$(eval which nasm)
export CC=$(eval which $TARGET-gcc)
export LD=$(eval which $TARGET-ld)

export AFLAGS="-f elf32"
export CFLAGS="-ffreestanding -fno-builtin -fno-stack-protector -nodefaultlibs -nostdlib -Wall -Wextra -Werror -std=c99 -I $INCDIR"
export LDFLAGS_BOOT="-m elf_i386 -T linkboot.ld"
export LDFLAGS_KERN="-m elf_i386 -T linkkern.ld"

$AS $AFLAGS -o bootstage1.asm.o x86/boot/src/bootstage1.asm 
$AS $AFLAGS -o bootstage2.asm.o x86/boot/src/bootstage2.asm 
$AS $AFLAGS -o a20.asm.o        x86/boot/src/a20.asm
$AS $AFLAGS -o bios.asm.o       x86/boot/src/bios.asm

$CC $CFLAGS -o bios.o   -c x86/boot/src/bios.c
$CC $CFLAGS -o diskio.o -c x86/boot/src/diskio.c
$CC $CFLAGS -o ram.o    -c x86/boot/src/ram.c
$CC $CFLAGS -o e820.o   -c x86/boot/src/e820.c
$CC $CFLAGS -o vbe.o    -c x86/boot/src/vbe.c
$CC $CFLAGS -o info.o   -c x86/boot/src/info.c

$LD $LDFLAGS_BOOT -o bootloader.bin *.o

rm *.o

$AS $AFLAGS -o start.asm.o       x86/kernel/src/start.asm
$AS $AFLAGS -o paging.asm.o      x86/kernel/src/paging.asm
$AS $AFLAGS -o gdt.asm.o         x86/kernel/src/gdt.asm
$AS $AFLAGS -o idt.asm.o         x86/kernel/src/idt.asm
$AS $AFLAGS -o interrupts.asm.o  x86/kernel/src/interrupts.asm

$CC $CFLAGS -c csupport/src/string.c
$CC $CFLAGS -c x86/drivers/src/keyboard.c
$CC $CFLAGS -c x86/drivers/src/pit.c
$CC $CFLAGS -c x86/drivers/src/pic.c
$CC $CFLAGS -c x86/kernel/src/welcome.c
$CC $CFLAGS -c x86/kernel/src/idt.c
$CC $CFLAGS -c x86/kernel/src/gdt.c
$CC $CFLAGS -c x86/kernel/src/paging.c -o paging_x86.o

$CC $CFLAGS -c kernel/src/machine.c
$CC $CFLAGS -c kernel/src/avos.c
$CC $CFLAGS -c kernel/src/paging.c
$CC $CFLAGS -c kernel/src/memory.c
$CC $CFLAGS -c kernel/src/process.c
$CC $CFLAGS -c kernel/src/interrupts.c
$CC $CFLAGS -c kernel/src/timer.c
$CC $CFLAGS -c kernel/src/drivers.c
$CC $CFLAGS -c kernel/src/ioports.c

$LD $LDFLAGS_KERN -o kernel.bin start.asm.o machine.o avos.o memory.o process.o paging.asm.o paging_x86.o paging.o gdt.o gdt.asm.o idt.o idt.asm.o interrupts.asm.o interrupts.o pic.o welcome.o pit.o timer.o keyboard.o drivers.o string.o ioports.o

rm *.o

dd conv=notrunc if=kernel.bin     of=avos.iso seek=64
dd conv=notrunc if=bootloader.bin of=avos.iso

rm *.bin
