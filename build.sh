#!/bin/bash

nasm -f bin   -o bootloader.bin    x86/boot/src/bootloader.asm
nasm -f bin   -o avosloader.bin    x86/boot/src/avosloader.asm 

nasm -f elf32 -o start.o           x86/kernel/src/start.asm
nasm -f elf32 -o gdt_asm.o         x86/kernel/src/gdt.asm
nasm -f elf32 -o interrupts_asm.o  x86/kernel/src/interrupts.asm

gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c csupport/src/string.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/drivers/src/keyboard.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/welcome.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/irqs.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/pic.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/interrupts.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/idt.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/tss.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/gdt.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/paging.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/checks.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/physmem.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c x86/kernel/src/kinit.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/kernel.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/dispensary.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/memory.c
gcc --target=i386-jos-elf -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nodefaultlibs -Wall -Wextra -Werror -std=c99 -I /Users/avartak/AVOS/AVOS -c kernel/src/heap.c

i386-elf-ld -m elf_i386 -T link.ld  -o kernel.bin  start.o kinit.o kernel.o physmem.o dispensary.o memory.o heap.o checks.o paging.o gdt.o gdt_asm.o tss.o idt.o interrupts_asm.o interrupts.o pic.o irqs.o welcome.o keyboard.o string.o

dd conv=notrunc if=kernel.bin     of=avos.flp seek=64
dd conv=notrunc if=avosloader.bin of=avos.flp seek=1
dd conv=notrunc if=bootloader.bin of=avos.flp

rm *.bin *.o
