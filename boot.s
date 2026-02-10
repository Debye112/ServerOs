.global _start
.extern kernel_main

_start:
    mov sp, #0x00010000    @ simple safe stack pointer (64 KB)
    bl kernel_main

hang:
    b hang
