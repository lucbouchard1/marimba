section .multiboot_header
header_start:
   dd 0xE85250D6                 ; magic number (multiboot 2)
   dd 0                          ; architecture 0 (protected mode i386)
   dd header_end - header_start  ; header length
   ; checksum
   dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

   ; end tag
   dw 0
   dw 0
   dd 8
header_end: