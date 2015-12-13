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

_data_start = ORIGIN(RAM);
_data_end = ORIGIN(RAM) + LENGTH(RAM);
_estack = ORIGIN(RAM) + LENGTH(RAM);

_Min_Heap_Size = 0;      /* required amount of heap  */
_Min_Stack_Size = 0x100; /* required amount of stack */

SECTIONS
{
    .text :
    {
        KEEP(*(isr_vectors))
        *(.text.main)
        KEEP(*(.text.*))
        /*
        *(.text.*)
        KEEP(*(.text.high))
        KEEP(*(.text.low))
        /**/
    } >FLASH
}
