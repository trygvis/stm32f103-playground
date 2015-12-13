cmake_minimum_required(VERSION 3.2)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/stm32.toolchain.cmake")

project(teensy-playground C CXX ASM)

#include(cmake/Teensy.cmake)

add_executable(test1.elf test1.cpp init.s include/stm32f10x_conf.h
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
        )

target_include_directories(test1.elf PUBLIC
        /usr/include/newlib
        include
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/CoreSupport
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/inc)
target_compile_definitions(test1.elf PUBLIC STM32F10X_MD USE_STDPERIPH_DRIVER)
include_directories(/usr/include/newlib)
#target_compile_options(test1 PRIVATE -nostartfiles)

target_compile_options(test1.elf PUBLIC "-O0")
set_target_properties(test1.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")

add_custom_command(TARGET test1.elf POST_BUILD
        COMMAND arm-none-eabi-objdump -D test1.elf > test1.elf.asm)
add_custom_command(TARGET test1.elf POST_BUILD
        COMMAND arm-none-eabi-nm test1.elf > test1.elf.nm)
add_custom_command(TARGET test1.elf POST_BUILD
        COMMAND arm-none-eabi-size test1.elf > test1.elf.size)
add_custom_command(TARGET test1.elf POST_BUILD
        COMMAND arm-none-eabi-readelf -a test1.elf > test1.elf.readelf)
add_custom_command(TARGET test1.elf POST_BUILD
        COMMAND arm-none-eabi-objcopy -O ihex test1.elf test1.elf.hex)
add_custom_command(TARGET test1.elf POST_BUILD
        COMMAND arm-none-eabi-objcopy -O binary test1.elf test1.elf.bin)
