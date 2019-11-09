#!/bin/bash

nasm -f bin   -o bootloader.bin    x86/boot/src/bootloader.asm
nasm -f bin   -o kload16.bin       x86/boot/src/kload16.asm 
nasm -f bin   -o kload32.bin       x86/boot/src/kload32.asm 

nasm -f elf32 -o start.o           kernel/src/start.asm
nasm -f elf32 -o gdt_asm.o         x86/kernel/src/gdt.asm
nasm -f elf32 -o interrupts_asm.o  x86/kernel/src/interrupts.asm

i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /home/avartak/AVOS/AVOS -c csupport/src/string.c
i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /home/avartak/AVOS/AVOS -c x86/drivers/src/keyboard.c
i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /home/avartak/AVOS/AVOS -c x86/kernel/src/welcome.c
i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /home/avartak/AVOS/AVOS -c x86/kernel/src/pic.c
i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /home/avartak/AVOS/AVOS -c x86/kernel/src/interrupts.c
i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /home/avartak/AVOS/AVOS -c x86/kernel/src/idt.c
i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /home/avartak/AVOS/AVOS -c x86/kernel/src/paging.c
i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /home/avartak/AVOS/AVOS -c x86/kernel/src/tss.c
i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /home/avartak/AVOS/AVOS -c x86/kernel/src/gdt.c
i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /home/avartak/AVOS/AVOS -c x86/kernel/src/kinit.c
i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /home/avartak/AVOS/AVOS -c kernel/src/kernel.c

i686-elf-ld -T link.ld  -o kernel.bin  start.o kernel.o kinit.o gdt.o gdt_asm.o tss.o paging.o idt.o interrupts_asm.o interrupts.o pic.o welcome.o keyboard.o string.o

dd conv=notrunc if=kernel.bin     of=avos.flp seek=40
dd conv=notrunc if=kload32.bin    of=avos.flp seek=24
dd conv=notrunc if=kload16.bin    of=avos.flp seek=8
dd conv=notrunc if=bootloader.bin of=avos.flp

rm *.bin *.o
