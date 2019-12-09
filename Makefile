TARGET=i686-elf
INCDIR=$(shell pwd)

CC=$(shell which $(TARGET)-gcc)
LD=$(shell which $(TARGET)-ld)
AS=$(shell which nasm)

ARCH=32
NASM_OUTPUT_FORMAT=elf$(ARCH)
LD_EMULATION=elf_i386

AFLAGS=-f $(NASM_OUTPUT_FORMAT)
CFLAGS=-ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Wall -Wextra -Werror -std=c99 -I $(INCDIR) -m$(ARCH) -T linkkern.ld -lgcc
LDFLAGS_BOOT=-m $(LD_EMULATION) -T linkboot.ld
LDFLAGS_KERN=-m $(LD_EMULATION) -T linkkern.ld

BOOT=boot/src
BOOT_OBJS=\
$(BOOT)/bootload32.s.o \
$(BOOT)/bios.s.o \
$(BOOT)/diskio.c.o \
$(BOOT)/memory.c.o  \
$(BOOT)/video.c.o  \
$(BOOT)/console.c.o  \
$(BOOT)/system.c.o  \
$(BOOT)/elf.c.o \
$(BOOT)/multiboot.c.o

CSUPPORT=csupport/src
CSUPPORT_OBJS=$(CSUPPORT)/string.c.o

X86_KERNEL=x86/kernel/src
X86_KERNEL_OBJS=\
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

modulelist.bin: boot/src/modulelist.asm
	$(AS) -f bin -o modulelist.bin boot/src/modulelist.asm

mbr.bin: boot/src/MBR.asm
	$(AS) -f bin -o mbr.bin boot/src/MBR.asm

vbr.bin: boot/src/VBR.asm
	$(AS) -f bin -o vbr.bin boot/src/VBR.asm

bootload16.bin: boot/src/bootload16.asm
	$(AS) -f bin -o bootload16.bin boot/src/bootload16.asm

bootload32.bin: $(BOOT_OBJS) $(CSUPPORT_OBJS)
	$(LD) $(LDFLAGS_BOOT) -o bootload32.bin  $(BOOT_OBJS) $(CSUPPORT_OBJS)

bootloader.bin: bootload16.bin bootload32.bin
	cat bootload16.bin bootload32.bin > bootloader.bin
	
kernel.bin: $(X86_KERNEL_OBJS) $(X86_DRIVERS_OBJS) $(KERNEL_OBJS) $(CSUPPORT_OBJS) $(CRT0) $(CRTI) $(CRTB) $(CRTE) $(CRTN)
	$(CC) $(CFLAGS) -o kernel.bin $(CRT0) $(CRTI) $(CRTB) $(X86_KERNEL_OBJS) $(X86_DRIVERS_OBJS) $(KERNEL_OBJS) $(CSUPPORT_OBJS) $(CRTE) $(CRTN)

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
	rm boot/src/*.o
	rm *.bin

