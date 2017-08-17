OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(vectors)

/* Highest address of the user mode stack */
_estack = 0x20010000;     /* end of RAM */


MEMORY
{
    ITCM2      : ORIGIN = 0x00000000, LENGTH = 0x10000
    DTCM2      : ORIGIN = 0x20000000, LENGTH = 0x10000
}

SECTIONS
{
    .vectors :
    {
        *(.isr_vectors)
        . = ALIGN(4);
    } > ITCM2

    .start :
    {
        *(.start*)
        . = ALIGN(4);
    } > ITCM2


    .text :
    {
        _text_start = . ;
        *(.text*)
        . = ALIGN(4);
    } > ITCM2

    __exidx_start = . ;
    .ARM.exidx :
    {
        *(.ARM.exidx*)
        . = ALIGN(4);
    } > ITCM2
    __exidx_end = . ;

    __extab_start = . ;
    .ARM.extab :
    {
        *(.ARM.extab*)
        . = ALIGN(4);
    } > ITCM2
    __extab_end = . ;

    _data_start = . ;

    .rodata :
    {
        *(.rodata*)
        . = ALIGN(4);
    } > DTCM2 AT> ITCM2

    .data :
    {
        *(.data*)
        . = ALIGN(4);
        *(.ramcode)
        . = ALIGN(4);
    } > DTCM2 AT> ITCM2

    .data_e :
    {
        _data_end = . ;
    } AT> ITCM2

    .bss :
    {
        _bss_start = . ;
        *(.bss*)
        . = ALIGN(4);
        *(COMMON)
        . = ALIGN(4);
        _bss_end = . ;
    } > DTCM2

    end = . ;
}


