/* linker.ld */
ENTRY(_start)

SECTIONS
{
    . = 0x8000; /* load address for versatilepb -kernel */
    
    .text : { *(.text*) }
    .rodata : { *(.rodata*) }
    .bss : { *(.bss*) }
    .data : { *(.data*) }
}
