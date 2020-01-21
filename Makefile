TARGET=i686-elf
INCDIR=$(shell pwd)

CC=$(shell which $(TARGET)-gcc)
LD=$(shell which $(TARGET)-ld)
AS=$(shell which nasm)

ARCH=32
NASM_OUTPUT_FORMAT=elf
LD_EMULATION=elf_i386

AFLAGS=-f $(NASM_OUTPUT_FORMAT)
CFLAGS=-ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Wall -Wextra -Werror -std=c11 -I $(INCDIR) -m$(ARCH) -T linkkern.ld -lgcc
LDFLAGS_BOOT=-m $(LD_EMULATION) -T linkboot.ld

BOOT=bootloader
BOOT_OBJS=\
$(BOOT)/initial/src/bootloader.s.o \
$(BOOT)/initial/src/a20.s.o \
$(BOOT)/multiboot/src/bios.s.o \
$(BOOT)/multiboot/src/diskio.c.o \
$(BOOT)/multiboot/src/memory.c.o  \
$(BOOT)/multiboot/src/video.c.o  \
$(BOOT)/multiboot/src/console.c.o  \
$(BOOT)/multiboot/src/system.c.o  \
$(BOOT)/multiboot/src/elf.c.o \
$(BOOT)/multiboot/src/multiboot.c.o \
$(BOOT)/multiboot/src/string.c.o

CLIB=kernel/clib
CLIB_OBJS=$(CLIB)/src/string.c.o

KERNEL=kernel
KERNEL_OBJS=\
$(KERNEL)/initial/src/multiboot.s.o \
$(KERNEL)/initial/src/preinit.s.o \
$(KERNEL)/initial/src/preinit.c.o \
$(KERNEL)/initial/src/welcome.c.o \
$(KERNEL)/arch/i386/src/functions.s.o \
$(KERNEL)/arch/i386/src/controlregs.s.o \
$(KERNEL)/arch/i386/src/gdt.s.o \
$(KERNEL)/arch/i386/src/gdt.c.o \
$(KERNEL)/arch/i386/src/interrupts.s.o \
$(KERNEL)/arch/i386/src/interrupts.c.o \
$(KERNEL)/arch/i386/src/flags.s.o \
$(KERNEL)/arch/i386/src/ioports.s.o \
$(KERNEL)/arch/apic/src/pic.c.o \
$(KERNEL)/arch/apic/src/apic.c.o

CRTB=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTE=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTI=$(KERNEL)/initial/src/crti.s.o
CRTN=$(KERNEL)/initial/src/crtn.s.o
CRT0=$(KERNEL)/initial/src/start.s.o

avos.iso: modulelist.bin kernel.bin bootloader.bin vbr.bin mbr.bin
	dd conv=notrunc if=modulelist.bin of=avos.iso seek=8192
	dd conv=notrunc if=kernel.bin of=avos.iso seek=4096
	dd conv=notrunc if=bootloader.bin of=avos.iso seek=2056
	dd conv=notrunc if=vbr.bin of=avos.iso seek=2048
	dd conv=notrunc if=mbr.bin of=avos.iso

mbr.bin: bootloader/initial/src/mbr.asm
	$(AS) -f bin -o mbr.bin bootloader/initial/src/mbr.asm

vbr.bin: bootloader/initial/src/vbr.asm
	$(AS) -f bin -o vbr.bin bootloader/initial/src/vbr.asm

bootloader.bin: $(BOOT_OBJS) $(CLIB_OBJS)
	$(LD) $(LDFLAGS_BOOT) -o bootloader.bin $(BOOT_OBJS)

kernel.bin: $(X86_KERNEL_OBJS) $(X86_DRIVERS_OBJS) $(KERNEL_OBJS) $(CLIB_OBJS) $(CRT0) $(CRTI) $(CRTB) $(CRTE) $(CRTN)
	$(CC) $(CFLAGS) -o kernel.bin $(CRT0) $(CRTI) $(CRTB) $(KERNEL_OBJS) $(CLIB_OBJS) $(CRTE) $(CRTN)

modulelist.bin: kernel/initial/src/modulelist.asm
	$(AS) -f bin -o modulelist.bin kernel/initial/src/modulelist.asm

%.s.o: %.asm
	$(AS) $(AFLAGS) -o $@ $<

%.c.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $< 

.PHONY: clean

clean:
	rm kernel/initial/src/*.o
	rm kernel/arch/i386/src/*.o
	rm kernel/arch/apic/src/*.o
	rm kernel/clib/src/*.o
	rm bootloader/initial/src/*.o
	rm bootloader/multiboot/src/*.o
	rm *.bin

