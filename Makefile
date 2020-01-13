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

CLIB=kernel/clib/src
CLIB_OBJS=$(CLIB)/string.c.o

X86_KERNEL=x86/kernel/src
X86_KERNEL_OBJS=\
$(X86_KERNEL)/multiboot.s.o \
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

CRTB=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTE=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTI=$(X86_KERNEL)/crti.s.o
CRTN=$(X86_KERNEL)/crtn.s.o
CRT0=$(X86_KERNEL)/start.s.o

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
	$(CC) $(CFLAGS) -o kernel.bin $(CRT0) $(CRTI) $(CRTB) $(X86_KERNEL_OBJS) $(X86_DRIVERS_OBJS) $(KERNEL_OBJS) $(CLIB_OBJS) $(CRTE) $(CRTN)

modulelist.bin: kernel/src/modulelist.asm
	$(AS) -f bin -o modulelist.bin kernel/src/modulelist.asm

%.s.o: %.asm
	$(AS) $(AFLAGS) -o $@ $<

%.c.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $< 

.PHONY: clean

clean:
	rm x86/drivers/src/*.o
	rm x86/kernel/src/*.o
	rm kernel/src/*.o
	rm kernel/clib/src/*.o
	rm bootloader/initial/src/*.o
	rm bootloader/multiboot/src/*.o
	rm *.bin

