# Bootloader
AVOS uses its own bootloader that has been written from scratch. The bootloader was the starting point of this project, and therefore, is the most advanced in terms of completeness. It is designed to be compliant with the [Multiboot2 specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html) but with certain caveats.

   ## Current Scope ##

   * The bootloader does not support UEFI. It has been designed to run on a BIOS supported machine. The aim is to eventually support UEFI, but only after other aspects of the OS (mainly the kernel) get sufficient timeand attention. 
   * Network support is not included
   * There is no support for reading any filesystems in the bootloader. The OS and other essential modules are loaded through a block list. 
   
  ## Overview of the bootstrap process ##

   * The execution starts with the 512 byte long [Master Boot Record](https://github.com/avartak/AVOS/blob/master/bootloader/initial/src/mbr.asm) or the *MBR*. The MBR identifies the *active partition* from its partition table, which contains our OS. The MBR relocates and then replaces itself with the code in the [Volume Boot Record](https://github.com/avartak/AVOS/blob/master/bootloader/initial/src/vbr.asm) or the *VBR* of the active partition. The VBR is also 512 bytes long.
   * The VBR is the first OS-specific piece of code. It's job is to load the bootloader binary code from disk into memory and start executing it. The first 128 bytes of the VBR contain a block list of disk sectors containing the bootloader binary. The rest of the VBR contains code to read these sectors from disk. Note that the VBR only supports BIOS *extensions* that can read from disk using the LBA scheme. These should be available on all modern PCs (and we are not aiming to support ancient machines and BIOSes which don't support them).
   * Now we come to the meat of the [bootloader](https://github.com/avartak/AVOS/blob/master/bootloader/initial/src/bootloader.asm) itself. The first 128 bytes of the bootloader are again reserved for a block list. This block list corresponds to sectors on disk containing the *module list* file. The module list file contains block lists of all the modules (the kernel, other auxiliary modules) that the bootloader is expected to load into memory. 
   * The bootloader code starts with some basic house-keeping. It enables the [A20 line](https://github.com/avartak/AVOS/blob/master/bootloader/initial/src/a20.asm) so that we have access to memory beyond 1 MB. The bootloader then enables the protected mode, launching the system into 32-bit mode. 
   * Next, the bootloader creates certain data structures that the OS needs to initialize itself. For instance, it stores a map of physical memory available in the system. The format in which this information is stored and made available to the OS is defined in the Multiboot2 specification. The corresponding code can be found in the [bootloader/multiboot](https://github.com/avartak/AVOS/tree/master/bootloader/multiboot) folder.
   * The bootloader loads the OS binary, and all requisite modules into memory and transfers control to the OS. The bootloader is capable of reading an OS binary in [ELF format](https://refspecs.linuxfoundation.org/elf/elf.pdf). It can also load a binary *as is*. 

   ## Block lists ##

The bootloader uses *blocklists* to identify disk sectors corresponding to certain files (bootloader or kernel binaries, auxiliary modules, a file containing blocklists). 

   * The block list structure in the VBR (and also at the start of the bootloader binary) is 128 bytes long.
   * The first 4 bytes contain the *JMP* instruction to jump past the block list, into executable code that follows the blocklist. 
   * The next 124 bytes contain data
   * The first 16 bytes out of these 124 bytes contain some load information. The first 8 bytes contain the 64-bit memory start address where the data/code from the blocklist is to be loaded. The next two bytes contain the sector size used by the disk (512 byte sectors are typical, but 4096 byte sectors are also coming about). The next 6 bytes are reserved for future use. 
   * The remaining 108 bytes contain the blocklist itself. A *block* is 12 bytes long, and so the blocklist can contain up to 9 blocks. The blocklist needs to terminate with a *NULL* block i.e. a block in which all the 12 bytes are 0. Therfore, the blocklist can have at most 8 blocks pointing to actual data on disk.
   * The first 8 bytes of a clock contain the 64-bit LBA of the start sector of a contiguous block of sectors. The next 4 bytes contain the size of the block, i.e. the number of contiguous sectors to be read out.

The blocklist in the *module list* is 512 bytes long. There is one block list for each module that the bootloader is expected to load. 

   * The first 4 bytes again carry the *JMP* instruction
   * The next 16 bytes carry load information similar to the 128 byte blocklist. In the case of the kernel binary, the reserved 6 bytes contain the characters 'K', 'E', 'R', 'N', 'E', 'L' in ASCII format. 
   * The remaining 492 bytes allow for up to 41 blocks. With a NULL terminated block, there can be as many as 40 blocks for a give file on disk. 
