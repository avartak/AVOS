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

X86_BOOT=x86/boot/src
X86_BOOT_OBJS=\
$(X86_BOOT)/bootstage1.s.o \
$(X86_BOOT)/bootstage2.s.o \
$(X86_BOOT)/a20.s.o \
$(X86_BOOT)/bios.s.o \
$(X86_BOOT)/bios.c.o \
$(X86_BOOT)/diskio.s.o \
$(X86_BOOT)/diskio.c.o \
$(X86_BOOT)/ram.c.o  \
$(X86_BOOT)/e820.c.o \
$(X86_BOOT)/vbe.s.o  \
$(X86_BOOT)/vbe.c.o  \
$(X86_BOOT)/elf.c.o  \
$(X86_BOOT)/multiboot.c.o 

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

avos.iso: kernel.bin bootloader.bin
	dd conv=notrunc if=kernel.bin of=avos.iso seek=64
	dd conv=notrunc if=bootloader.bin of=avos.iso

bootloader.bin: $(X86_BOOT_OBJS) $(CSUPPORT_OBJS)
	$(LD) $(LDFLAGS_BOOT) -o bootloader.bin  $(X86_BOOT_OBJS) $(CSUPPORT_OBJS)

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
	rm x86/boot/src/*.o
	rm *.bin

