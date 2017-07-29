OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(vectors)

/* Highest address of the user mode stack */
_estack = 0x20078000;     /* end of RAM */

MEMORY
{
    ITCM0      : ORIGIN = 0x00000000, LENGTH = 0x80000
    DTCM0      : ORIGIN = 0x20000000, LENGTH = 0x78000
}

SECTIONS
{
    .vectors :
    {
        *(.isr_vectors)
        . = ALIGN(4);
    } > ITCM0

    .start :
    {
        *(.start*)
        . = ALIGN(4);
    } > ITCM0

    .text :
    {
        *(.text*)
        . = ALIGN(4);
    } > ITCM0

    __exidx_start = . ;
    .ARM.exidx :
    {
        *(.ARM.exidx*)
        . = ALIGN(4);
    } > ITCM0
    __exidx_end = . ;

    __extab_start = . ;
    .ARM.extab :
    {
        *(.ARM.extab*)
        . = ALIGN(4);
    } > ITCM0
    __extab_end = . ;

    _data_start = . ;

    .rodata :
    {
        *(.rodata*)
        . = ALIGN(4);
    } > DTCM0 AT> ITCM0

    .data :
    {
        *(.data*)
        . = ALIGN(4);
        *(.ramcode)
        . = ALIGN(4);
    } > DTCM0 AT> ITCM0

    .data_e :
    {
        _data_end = . ;
    } AT> ITCM0

    .bss :
    {
        _bss_start = .;
        *(.bss*)
        . = ALIGN(4);
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > DTCM0

    end = . ;
}


