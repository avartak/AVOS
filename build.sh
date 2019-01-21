#!/bin/bash

nasm -f bin   -o bootloader.bin x86/boot/src/bootloader.asm
nasm -f bin   -o kload16.bin    x86/boot/src/kload16.asm 
nasm -f bin   -o kload32.bin    x86/boot/src/kload32.asm 

nasm -f elf32 -o start.o        kernel/src/start.asm
nasm -f elf32 -o gdt_asm.o      x86/kernel/src/gdt.asm
nasm -f elf32 -o idt_asm.o      x86/kernel/src/idt.asm

gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -D BINARY_IMAGE -I /Users/avartak/AVOS/AVOS -c csupport/src/string.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -D BINARY_IMAGE -I /Users/avartak/AVOS/AVOS -c x86/drivers/src/keyboard.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -D BINARY_IMAGE -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/welcome.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -D BINARY_IMAGE -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/pic.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -D BINARY_IMAGE -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/interrupts.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -D BINARY_IMAGE -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/idt.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -D BINARY_IMAGE -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/paging.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -D BINARY_IMAGE -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/gdt.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -D BINARY_IMAGE -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/kinit.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -D BINARY_IMAGE -I /Users/avartak/AVOS/AVOS -c kernel/src/kernel.c

i386-elf-ld -m elf_i386 -T link.ld  -o kernel.bin  start.o kernel.o kinit.o gdt.o gdt_asm.o paging.o idt.o idt_asm.o interrupts.o pic.o welcome.o keyboard.o string.o

dd conv=notrunc if=kernel.bin     of=avos.flp seek=40
dd conv=notrunc if=kload32.bin    of=avos.flp seek=24
dd conv=notrunc if=kload16.bin    of=avos.flp seek=8
dd conv=notrunc if=bootloader.bin of=avos.flp

rm *.bin *.o
