# AVOS
This is a hobby operating system currently under development. Work on this OS happens in my spare time. The goal is to learn about OS design, and all the details about coding an operating system into existence. The first goal is to have an OS targeting a 32-bit SMP x86 machine. Work on the kernel is only just starting, however, the bootloader is by now in a reasonably good shape.  

# Bootloader
AVOS uses its own bootloader that has been written from scratch. The bootloader was the starting point of this project, and therefore, is the most advanced in terms of completeness. It is designed to be compliant with the [Multiboot2 specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html) but with certain caveats.

   ## Current Scope ##

   * The bootloader does not support UEFI. It has been designed to run on a BIOS supported machine. The aim is to eventually support UEFI, but only after other aspects of the OS (mainly the kernel) get sufficient timeand attention. 
   * There is no support for any kind of networking information in the bootloader. 
   * There is no support for reading any filesystems in the bootloader. The OS and other essential modules are loaded through a block list. Support for at least the native filesystem of the OS (whatever that might be) is envisioned for the future. 
   * Several information structures (in some sense the services provided by the bootloader to the OS) are in place but are yet to be extensively tested (e.g. VBE and framebuffer information). The bootloader in general could benefit from more intensive testing. Hopefully, that can happen in the coming times.

The bootloader code can be found in the [bootloader/](https://github.com/avartak/AVOS/tree/master/bootloader) folder. The bootstrap process works as follows.

  ## Overview of the bootloader ##

   * The execution starts with the 512 byte long [Master Boot Record](https://github.com/avartak/AVOS/blob/master/bootloader/initial/src/mbr.asm) or the *MBR*. The MBR identifies the *active partition* from its partition table, which contains our OS. The MBR relocates and then replaces itself with the code in the [Volume Boot Record](https://github.com/avartak/AVOS/blob/master/bootloader/initial/src/vbr.asm) or the *VBR* of the active partition. The VBR is also 512 bytes long.
   * The VBR is the first OS-specific piece of code (the MBR can do the same job for any other BIOS compliant operating system). It's job is to load the bootloader binary code from disk into memory and start executing it. The first 128 bytes of the VBR contain a block list of disk sectors containing the bootloader binary. The rest of the VBR contains code to read these sectors from disk. Note that the VBR only supports BIOS *extensions* that can read from disk using the LBA scheme. These should be available on all modern PCs (and we are not aiming to support ancient machines and BIOSes which don't support them).
   * Now we come to the meat of the [bootloader](https://github.com/avartak/AVOS/blob/master/bootloader/initial/src/bootloader.asm) itself. The first 128 bytes of the bootloader are agin reserved for a block list. This block list corresponds to sectors on disk containing the *module list* file. The module list file contains block lists of all the modules (the kernel, other auxiliary modules) that the bootloader is expected to load into memory. 
   * The bootloader code starts with some basic house-keeping. It enables the [A20 line](https://github.com/avartak/AVOS/blob/master/bootloader/initial/src/a20.asm) so that we have access to memory beyond 1 MB. The bootloader then enables the protected mode, launching the system into 32-bit mode. 
   * Next, the bootloader creates certain data structures that the OS needs to initialize itself. For instance, it stores a map of physical memory available in the system. The format in which this information is stored and made available to the OS is defined in the Multiboot2 specification. 
   * The bootloader loads the OS binary, and all requisite modules into memory and transfers control to the OS. The bootloader is capable of reading an OS binary in [ELF format](https://refspecs.linuxfoundation.org/elf/elf.pdf). It can also load a binary *as is*. 


