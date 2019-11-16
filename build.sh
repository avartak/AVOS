#!/bin/bash

TARGET="i686-elf"
INCDIR="$PWD"
ARCH=32

AS=$(eval which nasm)
CC=$(eval which $TARGET-gcc)
LD=$(eval which $TARGET-ld)

NASM_OUTPUT_FORMAT=elf
LD_EMULATION=elf_i386
if [ $TARGET == "i686-elf" ]
then 
	NASM_OUTPUT_FORMAT="$NASM_OUTPUT_FORMAT$ARCH"
fi

if [ $TARGET == "x86_64-elf" ]
then
	if [ $ARCH -eq 32 ]
	then 
    	NASM_OUTPUT_FORMAT="$NASM_OUTPUT_FORMATx$ARCH"
		LD_EMULATION="elf32_x86_64"
	fi
	if [ $ARCH -eq 64 ]
	then 
    	NASM_OUTPUT_FORMAT="$NASM_OUTPUT_FORMAT$ARCH"
		LD_EMULATION="elf_x86_64"
	fi
fi

AFLAGS="-f $NASM_OUTPUT_FORMAT"
CFLAGS="-ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Wall -Wextra -Werror -std=c99 -I $INCDIR"
CFLAGS_LINK="-ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Wall -Wextra -Werror -std=c99 -I $INCDIR -m$ARCH -T linkkern.ld -lgcc"
LDFLAGS_BOOT="-m $LD_EMULATION -T linkboot.ld"
LDFLAGS_KERN="-m $LD_EMULATION -T linkkern.ld"

$AS -f bin  -o mbr.bin          x86/boot/src/mbr.asm
$AS -f bin  -o vbr.bin          x86/boot/src/vbr.asm
$AS -f bin  -o bootload16.bin   x86/boot/src/bootload16.asm 

$AS $AFLAGS -o bootload32.asm.o x86/boot/src/bootload32.asm 
$AS $AFLAGS -o a20.asm.o        x86/boot/src/a20.asm
$AS $AFLAGS -o bios.asm.o       x86/boot/src/bios.asm
$AS $AFLAGS -o diskio.asm.o     x86/boot/src/diskio.asm
$AS $AFLAGS -o vbe.asm.o        x86/boot/src/vbe.asm

$CC $CFLAGS -o string.o    -c csupport/src/string.c
$CC $CFLAGS -o bios.o      -c x86/boot/src/bios.c
$CC $CFLAGS -o diskio.o    -c x86/boot/src/diskio.c
$CC $CFLAGS -o ram.o       -c x86/boot/src/ram.c
$CC $CFLAGS -o vbe.o       -c x86/boot/src/vbe.c
$CC $CFLAGS -o elf.o       -c x86/boot/src/elf.c
$CC $CFLAGS -o io.o        -c x86/boot/src/io.c
$CC $CFLAGS -o discovery.o -c x86/boot/src/discovery.c
$CC $CFLAGS -o multiboot.o -c x86/boot/src/multiboot.c

$LD $LDFLAGS_BOOT -o bootload32.bin *.o

rm *.o
cat bootload16.bin bootload32.bin > bootloader.bin

$AS $AFLAGS -o start.asm.o       x86/kernel/src/start.asm
$AS $AFLAGS -o crti.o            x86/kernel/src/crti.asm
$AS $AFLAGS -o crtn.o            x86/kernel/src/crtn.asm
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

CRTB=$(eval $CC $CFLAGS -print-file-name=crtbegin.o)
CRTE=$(eval $CC $CFLAGS -print-file-name=crtend.o)
CRTI=crti.o
CRTN=crtn.o
CRT0=start.asm.o

$CC $CFLAGS_LINK -o kernel.bin $CRT0 $CRTI $CRTB machine.o avos.o memory.o process.o paging.asm.o paging_x86.o paging.o gdt.o gdt.asm.o idt.o idt.asm.o interrupts.asm.o interrupts.o pic.o welcome.o pit.o timer.o keyboard.o drivers.o string.o ioports.o $CRTE $CRTN

rm *.o

dd conv=notrunc if=kernel.bin     of=avos.iso seek=4096
dd conv=notrunc if=bootloader.bin of=avos.iso seek=2052
dd conv=notrunc if=vbr.bin        of=avos.iso seek=2048
dd conv=notrunc if=mbr.bin        of=avos.iso

rm *.bin
