# Getting started

The 1-2-3 guide:

    mkdir build
    cd build
    cmake ..
    make

and it should build a set of imaged you can flash.

# TODOs

o Upgrade to cmake 3.6 which gives some improvements and fixes this:

    The CMAKE_FORCE_C_COMPILER macro is deprecated.  Instead just set CMAKE_C_COMPILER and allow CMake to identify the compiler.

o Add cmake dependency/find_package on libelf.
# Resources

* http://www.downloads.seng.de/HowTo_ToolChain_STM32_Ubuntu.pdf
* http://embedded.kleier.selfhost.me/lockup.php
* http://fun-tech.se/stm32/linker/index.php
* Developing a Generic Hard Fault handler for ARM Cortex-M3/Cortex-M4: https://community.arm.com/servlet/JiveServlet/previewBody/7835-102-2-12371/Developing%20a%20Generic%20Hard%20Fault%20handler%20for%20ARM.pdf
* Schematic: http://img.banggood.com/file/products/20150205235330SKU120191.pdf
* http://www.st.com/web/en/resource/technical/document/datasheet/CD00161566.pdf
* http://www.banggood.com/ARM-Cortex-M3-STM32F103C8T6-STM32-Minimum-System-Development-Board-p-920184.html
* http://www.lctech-inc.com/Hardware/Detail.aspx?id=0172e854-77b0-43d5-b300-68e570c914fd
* https://github.com/dwelch67/stm32_samples
* http://stackoverflow.com/questions/9565921/cortex-m3-initialisation

* http://stackoverflow.com/questions/32422075/arm-bare-metal-binary-linking
* https://github.com/ckormanyos/real-time-cpp/blob/master/ref_app/target/micros/stm32f100/make/stm32f100.ld
* http://www.bravegnu.org/gnu-eprog/c-startup.html

* http://www.st.com/web/en/catalog/tools/PF257890
** STSW-STM32054	

# Programming with OpenOCD

    reset halt
    flash probe 0
    stm32f1x mass_erase 0
    flash write_bank 0 test1.elf.bin 0
    reset run

# Registers

## CPU Registers

The Stack Pointer (SP) is register R13. In Thread mode, bit[1] of the CONTROL register indicates the stack pointer to use:
0 = Main Stack Pointer (MSP). This is the reset value.
1 = Process Stack Pointer (PSP).

## Configurable Fault Status Register: 0xE000ED28


## Hard Fault Status Register: 0xE000ED2C

Table 4.28. HFSR bit assignments
Bits	Name	Function
[31]	DEBUGEVT	Reserved for Debug use. When writing to the register you must write 0 to this bit, otherwise behavior is Unpredictable.
[30]	FORCED	
Indicates a forced hard fault, generated by escalation of a fault with configurable priority that cannot be handles, either because of priority or because it is disabled:
0 = no forced HardFault
1 = forced HardFault.
When this bit is set to 1, the HardFault handler must read the other fault status registers to find the cause of the fault.
[29:2]	-	Reserved.
[1]	VECTTBL	
Indicates a BusFault on a vector table read during exception processing:
0 = no BusFault on vector table read
1 = BusFault on vector table read.
This error is always handled by the hard fault handler.
When this bit is set to 1, the PC value stacked for the exception return points to the instruction that was preempted by the exception.
[0]	-	Reserved.

### Faults

    HFSR = 0x40000000 => FORCED, double error, check the other flags

## Debug Fault Status Register: 0xE000ED30

## Configurable Fault Status Register: 0xE000ED28

The following subsections describe the subregisters that make up the CFSR:
* MemManage Fault Status Register
* BusFault Status Register
* UsageFault Status Register.

    CFSR = 0x00020000 => INVSTATE (Invalid state)

* [1] INVSTATE	Invalid state UsageFault:
    * 0 = no invalid state UsageFault
    * 1 = the processor has attempted to execute an instruction that makes illegal use of the EPSR.
    * When this bit is set to 1, the PC value stacked for the exception return points to the instruction that attempted the illegal use of the EPSR.
    * This bit is not set to 1 if an undefined instruction uses the EPSR.

## Auxiliary Fault Status Register: 0xE000ED3C

# GDB Tips

## Stack straces

Show stack trace:

    (gdb) bt
    #0  0x08000050 in HardFault_Handler ()
    #1  <signal handler called>
    #2  0x08000044 in _Reset_Handler ()

Show details on each frame:

    (gdb) info frame 0
    Stack frame at 0x20000fe0:
     pc = 0x8000050 in HardFault_Handler; saved pc = 0xfffffff9
     called by frame at 0x20001000
     Arglist at 0x20000fe0, args:
     Locals at 0x20000fe0, Previous frame's sp is 0x20000fe0
