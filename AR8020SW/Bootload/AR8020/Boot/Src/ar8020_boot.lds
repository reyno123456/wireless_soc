OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(vectors)

/* Highest address of the user mode stack */
_estack = 0x20080000;

MEMORY 
{
    FLASH : ORIGIN = 0x10000000, LENGTH = 0x1000
    DTCM0 : ORIGIN = 0x20000000, LENGTH = 0x80000
}

SECTIONS
{
    .isr_vectors :
    {
        _vectors_start = . ;
        *(.isr_vectors)
        _vectors_end = . ;
    } > FLASH

    .start :
    {
        *(.start.*)
    } > FLASH

    .text : 
    {
        _text_start = . ;
        *(.text*)
        . = ALIGN(4) ;
    } > FLASH

    _data_start = .;
    
    .rodata :
    {
        *(.rodata*)
        . = ALIGN(4) ;
    }  >DTCM0 AT> FLASH

    .data : 
    {
        *(.data*)
        . = ALIGN(4) ;
        *(.ramcode)
    } >DTCM0 AT> FLASH

    .data_e :
    {
        _data_end = . ;
    } AT> FLASH

    .bss :
    {
        _bss_start = . ;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > DTCM0

    end = . ;
}


