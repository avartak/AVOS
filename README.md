# AVOS
This is a hobby operating system currently under development. Work on this OS happens in my spare time. The goal is to learn about OS design, and all the details about coding an operating system into existence. The first goal is to have an OS targeting a 32-bit SMP x86 machine. Work on the kernel is only just starting, however, the bootloader is by now in a reasonably good shape.  

# Bootloader
AVOS uses its own bootloader that has been written from scratch. The bootloader was the starting point of this project, and therefore, is the most advanced in terms of completeness. It is designed to be compliant with the [Multiboot2 specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html) but with certain caveats.

   * The bootloader does not support UEFI. It has been designed to run on a BIOS supported machine. The aim is to eventually support UEFI, but only after other aspects of the OS (mainly the kernel) get sufficient timeand attention. 
   * There is no support for any kind of networking information in the bootloader. 
   * There is no support for reading any filesystems in the bootloader. The OS and other essential modules are loaded through a block list. Support for at least the native filesystem of the OS (whatever that might be) is envisioned for the future. 
   * Several information structures (in some sense the services provided by the bootloader to the OS) are in place but are yet to be extensively tested (e.g. VBE and framebuffer information). The bootloader in general could benefit from more intensive testing. Hopefully, that can happen in the coming times.

The bootloader code can be found in the [bootloader/](https://github.com/avartak/AVOS/tree/master/bootloader) folder. The bootstrap process starts with the [Master Boot Record](https://github.com/avartak/AVOS/blob/master/bootloader/initial/src/mbr.asm) or the *MBR*.  
