cmake_minimum_required(VERSION 3.2)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/stm32.toolchain.cmake")

project(stm32f103-playground C CXX ASM)

function(add_extra_commands target_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND arm-none-eabi-objdump -D ${target_name} > ${target_name}.asm)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND arm-none-eabi-nm ${target_name} > ${target_name}.nm)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND arm-none-eabi-size ${target_name} > ${target_name}.size)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND arm-none-eabi-readelf -a ${target_name} > ${target_name}.readelf)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND arm-none-eabi-objcopy -O ihex ${target_name} ${target_name}.hex)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND arm-none-eabi-objcopy -O binary ${target_name} ${target_name}.bin)
endfunction()

#########################################################################################################
# test1

add_executable(test1.elf test1.cpp init_low.s init_high.cpp include/stm32f10x_conf.h
        # http://www.sparetimelabs.com/tinyprintf/tinyprintf.php
        tmp/printf/printf.h tmp/printf/printf.c
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
        )

target_include_directories(test1.elf PUBLIC
        include
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/CoreSupport
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/inc)
target_compile_definitions(test1.elf PUBLIC STM32F10X_MD USE_STDPERIPH_DRIVER)

target_compile_options(test1.elf PUBLIC "-O0")
set_target_properties(test1.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")
add_extra_commands(test1.elf)

#########################################################################################################
# serial1

add_executable(serial1.elf serial1.cpp init_low.s init_high.cpp include/stm32f10x_conf.h
        debug.cpp debug.h
        # http://www.sparetimelabs.com/tinyprintf/tinyprintf.php
        tmp/printf/printf.h tmp/printf/printf.c
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
        )

target_include_directories(serial1.elf PUBLIC
        include
        tmp/printf
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/CoreSupport
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
        tmp/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/inc)
target_compile_definitions(serial1.elf PUBLIC STM32F10X_MD USE_STDPERIPH_DRIVER)

set_target_properties(serial1.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")
add_extra_commands(serial1.elf)
