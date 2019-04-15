#!/bin/bash

nasm -f elf32 -o bootstage1.o      x86/boot/src/bootstage1.asm 
nasm -f elf32 -o bootstage2.o      x86/boot/src/bootstage2.asm 
nasm -f elf32 -o a20.o             x86/boot/src/a20.asm
nasm -f elf32 -o bios.o            x86/boot/src/bios.asm

gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/boot/src/diskio.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/boot/src/ram.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/boot/src/e820.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/boot/src/vbe.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/boot/src/bootinfo.c

nasm -f elf32 -o start.o           x86/kernel/src/start.asm
nasm -f elf32 -o paging_asm.o      x86/kernel/src/paging.asm
nasm -f elf32 -o gdt_asm.o         x86/kernel/src/gdt.asm
nasm -f elf32 -o idt_asm.o         x86/kernel/src/idt.asm
nasm -f elf32 -o interrupts_asm.o  x86/kernel/src/interrupts.asm

gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c csupport/src/string.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/drivers/src/keyboard.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/drivers/src/pit.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/drivers/src/pic.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/welcome.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/idt.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/gdt.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/paging.c -o paging_x86.o

gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/machine.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/avos.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/paging.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/memory.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/process.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/interrupts.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/timer.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/drivers.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/ioports.c

i386-elf-ld -m elf_i386 -T linkboot.ld -o bootloader.bin bootstage1.o bootstage2.o a20.o vbe.o e820.o bios.o ram.o diskio.o bootinfo.o

i386-elf-ld -m elf_i386 -T linkkern.ld -o kernel.bin start.o machine.o avos.o memory.o process.o paging_asm.o paging_x86.o paging.o gdt.o gdt_asm.o idt.o idt_asm.o interrupts_asm.o interrupts.o pic.o welcome.o pit.o timer.o keyboard.o drivers.o string.o ioports.o

dd conv=notrunc if=kernel.bin     of=avos.iso seek=64
dd conv=notrunc if=bootloader.bin of=avos.iso

rm *.bin *.o
