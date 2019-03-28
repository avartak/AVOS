; The Multiboot-2 header has the following structure
; The tags (type = 1,2,3,....) are optional
; Note : Tags are terminated with a 'null tag' of type 0 and size 8
; We will only include tags 2 and 3 in our multiboot header

;                        +-------------------+
;    unsigned 32-bit     | magic             |
;    unsigned 32-bit     | architecture      |
;    unsigned 32-bit     | header_length     |
;    unsigned 32-bit     | checksum          |
;    unsigned 32-bit     | bss_end_addr      |
;                        +-------------------+
;
;                        +-------------------+
;    unsigned 16-bit     | type              |
;    unsigned 16-bit     | flags             |
;    unsigned 32-bit     | size              |
;                        +-------------------+
;
;                        +-------------------+
;    unsigned 16-bit     | type = 1          |
;    unsigned 16-bit     | flags             |
;    unsigned 32-bit     | size              |
;    unsigned 32-bit(n)  | mbi_tag_types     |
;                        +-------------------+
;
;                        +-------------------+
;    unsigned 16-bit     | type = 2          |
;    unsigned 16-bit     | flags             |
;    unsigned 32-bit     | size              |
;    unsigned 32-bit     | header_addr       |
;    unsigned 32-bit     | load_addr         |
;    unsigned 32-bit     | load_end_addr     |
;    unsigned 32-bit     | bss_end_addr      |
;                        +-------------------+
;
;                        +-------------------+
;    unsigned 16-bit     | type = 3          |
;    unsigned 16-bit     | flags             |
;    unsigned 32-bit     | size              |
;    unsigned 32-bit     | entry_addr        |
;                        +-------------------+
;
;                        +-------------------+
;    unsigned 16-bit     | type = 4          |
;    unsigned 16-bit     | flags             |
;    unsigned 32-bit     | size = 12         |
;    unsigned 32-bit     | console_flags     |
;                        +-------------------+
;
;                        +-------------------+
;    unsigned 16-bit     | type = 5          |
;    unsigned 16-bit     | flags             |
;    unsigned 32-bit     | size = 20         |
;    unsigned 32-bit     | width             |
;    unsigned 32-bit     | height            |
;    unsigned 32-bit     | depth             |
;                        +-------------------+
;
;                        +-------------------+
;    unsigned 16-bit     | type = 6          |
;    unsigned 16-bit     | flags             |
;    unsigned 32-bit     | size = 8          |
;                        +-------------------+
;
;                        +-------------------+
;    unsigned 16-bit     | type = 7          |
;    unsigned 16-bit     | flags             |
;    unsigned 32-bit     | size = 8          |
;                        +-------------------+
;
;                        +-------------------+
;    unsigned 16-bit     | type = 8          |
;    unsigned 16-bit     | flags             |
;    unsigned 32-bit     | size              |
;    unsigned 32-bit     | entry_addr        |
;                        +-------------------+
;
;                        +-------------------+
;    unsigned 16-bit     | type = 9          |
;    unsigned 16-bit     | flags             |
;    unsigned 32-bit     | size              |
;    unsigned 32-bit     | entry_addr        |
;                        +-------------------+
;
;                        +-------------------+
;    unsigned 16-bit     | type = 10         |
;    unsigned 16-bit     | flags             |
;    unsigned 32-bit     | size = 24         |
;    unsigned 32-bit     | min_addr          |
;    unsigned 32-bit     | max_addr          |
;    unsigned 32-bit     | align             |
;    unsigned 32-bit     | preference        |
;                        +-------------------+

MULTIBOOT2_HEADER_MAGIC      equ 0xE85250D6
MULTIBOOT2_HEADER_ARCH       equ 0
MULTIBOOT2_HEADER_SIZE       equ Multiboot_End - Multiboot_Start
MULTIBOOT2_HEADER_CHECKSUM   equ 0x100000000 -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT2_HEADER_ARCH + MULTIBOOT2_HEADER_SIZE)

MULTIBOOT2_TAG1_TAG          equ 0x1
MULTIBOOT2_TAG1_FLAGS        equ 0x0
MULTIBOOT2_TAG1_SIZE         equ 0xC
MULTIBOOT2_TAG1_REQ_TYPE6    equ 0x6

MULTIBOOT2_TAG2_TAG          equ 0x2
MULTIBOOT2_TAG2_FLAGS        equ 0x0
MULTIBOOT2_TAG2_SIZE         equ 0x18
MULTIBOOT2_TAG2_HEADER_ADDR  equ 0x100000
MULTIBOOT2_TAG2_LOAD_ADDR    equ -1 
MULTIBOOT2_TAG2_LOAD_END     equ 0
MULTIBOOT2_TAG2_BSS_END      equ 0

MULTIBOOT2_TAG3_TAG          equ 0x3
MULTIBOOT2_TAG3_FLAGS        equ 0x0
MULTIBOOT2_TAG3_SIZE         equ 0xC
MULTIBOOT2_TAG3_ENTRY        equ 0x100000+MULTIBOOT2_HEADER_SIZE

MULTIBOOT2_TAG0_TAG          equ 0x0
MULTIBOOT2_TAG0_FLAGS        equ 0x0
MULTIBOOT2_TAG0_SIZE         equ 0x8

Multiboot_Start:

	dd MULTIBOOT2_HEADER_MAGIC
	dd MULTIBOOT2_HEADER_ARCH
	dd MULTIBOOT2_HEADER_SIZE	
	dd MULTIBOOT2_HEADER_CHECKSUM

	dw MULTIBOOT2_TAG1_TAG
	dw MULTIBOOT2_TAG1_FLAGS
	dd MULTIBOOT2_TAG1_SIZE
	dd MULTIBOOT2_TAG1_REQ_TYPE6

	dw MULTIBOOT2_TAG2_TAG
	dw MULTIBOOT2_TAG2_FLAGS
	dd MULTIBOOT2_TAG2_SIZE
	dd MULTIBOOT2_TAG2_HEADER_ADDR	
	dd MULTIBOOT2_TAG2_LOAD_ADDR
	dd MULTIBOOT2_TAG2_LOAD_END
	dd MULTIBOOT2_TAG2_BSS_END

	dw MULTIBOOT2_TAG3_TAG
	dw MULTIBOOT2_TAG3_FLAGS
	dd MULTIBOOT2_TAG3_SIZE
	dd MULTIBOOT2_TAG3_ENTRY

	dw MULTIBOOT2_TAG0_TAG
	dw MULTIBOOT2_TAG0_FLAGS
	dd MULTIBOOT2_TAG0_SIZE

Multiboot_End:

global MultibootInfo
MultibootInfo :

	dd 0xFFFFFFFF
