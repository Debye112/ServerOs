/* linker.ld — Raspberry Pi 5 bare‑metal kernel */
ENTRY(_start)

SECTIONS
{
    /* Firmware loads kernel8.img at 0x80000 */
    . = 0x80000;

    /* Code section */
    .text : ALIGN(4096) {
        *(.text.boot)     /* boot.S */
        *(.text*)         /* all other code */
    }

    /* Read‑only data */
    .rodata : ALIGN(4096) {
        *(.rodata*)
    }

    /* Initialized data */
    .data : ALIGN(4096) {
        *(.data*)
    }

    /* Uninitialized data (zeroed at boot) */
    .bss : ALIGN(4096) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    /* Define stack at 32MB mark (safe region) */
    . = 0x2000000;
    _stack_top = .;
}

