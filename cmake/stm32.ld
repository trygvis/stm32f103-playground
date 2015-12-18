/*
 * https://github.com/barrybingo/SunMoonSim/blob/master/LinkerScript/STM32F103-MD.ld
 */

/* Entry Point */
ENTRY(_Reset_Handler)

/* Specify the memory areas */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 64K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 20k
  MEMORY_B1 (rx)  : ORIGIN = 0x60000000, LENGTH = 0K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .text :
    {
        *(.isr_vectors)
        init.s:(.text)
        *(.text)
        KEEP(*(.text.*))
        *(.rodata*)
    } >FLASH

     . = ORIGIN(RAM);

    .data ALIGN(4) :
    {
        *(.data*)
    } >RAM AT >FLASH

    .bss ALIGN(4) (NOLOAD) :
    {
        *(.bss)
        *(.bss.*)
    } >RAM

    _copy_data_store = ADDR(.data);
    _copy_data_store_end = _copy_data_store + SIZEOF(.data);
    _copy_data_load = LOADADDR(.data);

    _bss_start = ADDR(.bss);
    _bss_end = _bss_start + SIZEOF(.bss);
}
