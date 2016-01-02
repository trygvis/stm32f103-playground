cmake_minimum_required(VERSION 3.2)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/stm32.toolchain.cmake")

project(stm32f103-playground C CXX ASM)

include(ExternalProject)
ExternalProject_Add(host-utils
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/host"
        CONFIGURE_COMMAND cmake "-GUnix Makefiles" <SOURCE_DIR> -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        BUILD_COMMAND "make"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/host-utils"
        INSTALL_COMMAND make install
)

function(add_extra_commands target_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND mkdir -p ${target_name}-info && arm-none-eabi-objdump -D ${target_name} > ${target_name}-info/${target_name}.asm)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND mkdir -p ${target_name}-info && arm-none-eabi-nm ${target_name} > ${target_name}-info/${target_name}.nm)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND mkdir -p ${target_name}-info && arm-none-eabi-size ${target_name} > ${target_name}-info/${target_name}.size)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND mkdir -p ${target_name}-info && arm-none-eabi-readelf -a ${target_name} > ${target_name}-info/${target_name}.readelf)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND mkdir -p ${target_name}-info && arm-none-eabi-objcopy -O ihex ${target_name} ${target_name}-info/${target_name}.hex)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND mkdir -p ${target_name}-info && arm-none-eabi-objcopy -O binary ${target_name} ${target_name}-info/${target_name}.bin)

    add_custom_command(TARGET ${target_name} DEPENDS host-utils POST_BUILD
            COMMAND "${CMAKE_BINARY_DIR}/host-utils/bin/elfinfo" -f ${target_name} -t 0x08000000:20k -d 0x20000000:5k)
endfunction()

# https://github.com/cjlano/tinyprintf
add_library(tinyprintf STATIC tinyprintf/tinyprintf.c tinyprintf/tinyprintf.h)
target_include_directories(tinyprintf PUBLIC tinyprintf)

set(STM32F10X_STDPERIPH_LIB ${CMAKE_SOURCE_DIR}/tmp/STM32F10x_StdPeriph_Lib_V3.5.0)
set(STM32F10X_STDPERIPH_DEFINES STM32F10X_MD USE_STDPERIPH_DRIVER)

#########################################################################################################
# test1

add_executable(test1.elf test1.cpp init_low.s init_high.cpp include/init_high.h include/stm32f10x_conf.h
        # http://www.sparetimelabs.com/tinyprintf/tinyprintf.php
        tinyprintf/tinyprintf.c tinyprintf/tinyprintf.h
        debug.cpp include/debug.h
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
        )

target_include_directories(test1.elf PUBLIC
        include
        tinyprintf
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/inc)
target_compile_definitions(test1.elf PUBLIC ${STM32F10X_STDPERIPH_DEFINES})

target_compile_options(test1.elf PUBLIC "-O0")
set_target_properties(test1.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")
add_extra_commands(test1.elf)

#########################################################################################################
# serial1

add_executable(serial1.elf serial1.cpp init_low.s init_high.cpp include/init_high.h include/stm32f10x_conf.h
        debug.cpp include/debug.h
        # http://www.sparetimelabs.com/tinyprintf/tinyprintf.php
        tinyprintf/tinyprintf.c tinyprintf/tinyprintf.h
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
        )

target_include_directories(serial1.elf PUBLIC
        include
        tinyprintf
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/inc)
target_compile_definitions(serial1.elf PUBLIC ${STM32F10X_STDPERIPH_DEFINES})

set_target_properties(serial1.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")
add_extra_commands(serial1.elf)

#########################################################################################################
# serial2

add_executable(serial2.elf serial2.cpp init_low.s init_high.cpp include/init_high.h include/stm32f10x_conf.h
        include/playground.h
        debug.cpp include/debug.h
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/misc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
        )
target_link_libraries(serial2.elf tinyprintf)

target_include_directories(serial2.elf PUBLIC
        include
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/inc)
target_compile_definitions(serial2.elf PUBLIC ${STM32F10X_STDPERIPH_DEFINES})

set_target_properties(serial2.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")
add_extra_commands(serial2.elf)

#########################################################################################################
# os1

add_executable(os1.elf os1.cpp os1_cm3.s init_low.s init_high.cpp include/init_high.h include/stm32f10x_conf.h
        include/playground.h
        debug.cpp include/debug.h
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/misc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
        )
target_link_libraries(os1.elf tinyprintf)

target_include_directories(os1.elf PUBLIC
        include
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/inc)
target_compile_definitions(os1.elf PUBLIC ${STM32F10X_STDPERIPH_DEFINES})

set_target_properties(os1.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")
add_extra_commands(os1.elf)

#########################################################################################################
# os2

add_executable(os2.elf os2.cpp os2_cm3.s init_low.s init_high.cpp include/init_high.h include/stm32f10x_conf.h
        include/playground.h
        debug.cpp include/debug.h
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/misc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
        )
target_link_libraries(os2.elf tinyprintf)

target_include_directories(os2.elf PUBLIC
        include
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/inc)
target_compile_definitions(os2.elf PUBLIC ${STM32F10X_STDPERIPH_DEFINES})

set_target_properties(os2.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")
add_extra_commands(os2.elf)

add_subdirectory(apps)

##

add_library(playground OBJECT
        init_low.s init_high.cpp
        include/init_high.h

        include/stm32f10x_conf.h

        include/playground.h

        debug.cpp
        include/debug.h)
target_include_directories(playground PUBLIC
        include
        tinyprintf
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/inc
        )
target_compile_definitions(playground PUBLIC ${STM32F10X_STDPERIPH_DEFINES})
