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
            COMMAND mkdir -p ${target_name}-info && arm-none-eabi-nm -C ${target_name} > ${target_name}-info/${target_name}.nm)
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
set(PLAYGROUND_DIR ${CMAKE_SOURCE_DIR}/playground)
set(TINYPRINTF_DIR ${CMAKE_SOURCE_DIR}/tinyprintf)

add_library(playground OBJECT
        playground/src/init_low.s playground/src/init_high.cpp
        playground/include/init_high.h

        playground/include/stm32f10x_conf.h

        playground/include/playground.h

        playground/src/debug.cpp
        playground/include/debug.h)
target_include_directories(playground PUBLIC
        playground/include
        tinyprintf
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/inc
        )
target_compile_definitions(playground PUBLIC ${STM32F10X_STDPERIPH_DEFINES})

add_subdirectory(apps)
