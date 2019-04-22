TARGET=i686-elf
INCDIR=/Users/avartak/AVOS/AVOS

CC=/Users/avartak/AVOS/Compiler/Install/bin/i686-elf-gcc
LD=/Users/avartak/AVOS/Compiler/Install/bin/i686-elf-ld
AS=nasm

AFLAGS=-f elf32
CFLAGS=-ffreestanding -fno-builtin -fno-stack-protector -nodefaultlibs -nostdlib -Wall -Wextra -Werror -std=c99 -I $(INCDIR)
LDFLAGS_BOOT=-m elf_i386 -T linkboot.ld
LDFLAGS_KERN=-m elf_i386 -T linkkern.ld

CRTBEG=/Users/avartak/AVOS/Compiler/Install/lib/gcc/i686-elf/8.3.0/crtbegin.o
CRTEND=/Users/avartak/AVOS/Compiler/Install/lib/gcc/i686-elf/8.3.0/crtend.o

X86_BOOT=x86/boot/src
X86_BOOT_OBJS=\
$(X86_BOOT)/bootstage1.s.o \
$(X86_BOOT)/bootstage2.s.o \
$(X86_BOOT)/a20.s.o \
$(X86_BOOT)/bios.s.o \
$(X86_BOOT)/bios.c.o \
$(X86_BOOT)/diskio.c.o \
$(X86_BOOT)/ram.c.o  \
$(X86_BOOT)/e820.c.o \
$(X86_BOOT)/vbe.c.o  \
$(X86_BOOT)/info.c.o 

CSUPPORT=csupport/src
CSUPPORT_OBJS=$(CSUPPORT)/string.c.o

X86_KERNEL=x86/kernel/src
X86_KERNEL_OBJS=\
$(X86_KERNEL)/start.s.o \
$(X86_KERNEL)/paging.s.o \
$(X86_KERNEL)/gdt.s.o \
$(X86_KERNEL)/idt.s.o \
$(X86_KERNEL)/interrupts.s.o \
$(X86_KERNEL)/welcome.c.o \
$(X86_KERNEL)/idt.c.o \
$(X86_KERNEL)/gdt.c.o \
$(X86_KERNEL)/paging.c.o

X86_DRIVERS=x86/drivers/src
X86_DRIVERS_OBJS=\
$(X86_DRIVERS)/keyboard.c.o \
$(X86_DRIVERS)/pit.c.o \
$(X86_DRIVERS)/pic.c.o

KERNEL=kernel/src
KERNEL_OBJS=\
$(KERNEL)/machine.c.o \
$(KERNEL)/avos.c.o \
$(KERNEL)/paging.c.o \
$(KERNEL)/memory.c.o \
$(KERNEL)/process.c.o \
$(KERNEL)/interrupts.c.o \
$(KERNEL)/timer.c.o \
$(KERNEL)/drivers.c.o \
$(KERNEL)/ioports.c.o

avos.iso: kernel.bin bootloader.bin
	dd conv=notrunc if=kernel.bin of=avos.iso seek=64
	dd conv=notrunc if=bootloader.bin of=avos.iso

bootloader.bin: $(X86_BOOT_OBJS)
	$(LD) $(LDFLAGS_BOOT) -o bootloader.bin  $(X86_BOOT_OBJS)

kernel.bin: $(X86_KERNEL_OBJS) $(X86_DRIVERS_OBJS) $(KERNEL_OBJS) $(CSUPPORT_OBJS)
	$(LD) $(LDFLAGS_KERN) -o kernel.bin $(X86_KERNEL_OBJS) $(X86_DRIVERS_OBJS) $(KERNEL_OBJS) $(CSUPPORT_OBJS)

%.s.o: %.asm
	$(AS) $(AFLAGS) -o $@ $<

%.c.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $< 

.PHONY: clean

clean:
	rm x86/drivers/src/*.o
	rm x86/kernel/src/*.o
	rm kernel/src/*.o
	rm csupport/src/*.o
	rm x86/boot/src/*.o
	rm *.bin
