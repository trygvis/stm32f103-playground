add_executable(blinky.elf blinky.cpp
        ${PLAYGROUND_DIR}/src/init_low.s ${PLAYGROUND_DIR}/src/init_high.cpp ${PLAYGROUND_DIR}/include/init_high.h
        ${PLAYGROUND_DIR}/include/playground.h
        ${PLAYGROUND_DIR}/src/debug.cpp ${PLAYGROUND_DIR}/include/debug.h
        ${PLAYGROUND_DIR}/include/stm32f10x_conf.h
        # http://www.sparetimelabs.com/tinyprintf/tinyprintf.php
        ${TINYPRINTF_DIR}/tinyprintf.c ${TINYPRINTF_DIR}/tinyprintf.h
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
        )

target_include_directories(blinky.elf PUBLIC
        ${PLAYGROUND_DIR}/include
        ${TINYPRINTF_DIR}
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/inc)
target_compile_definitions(blinky.elf PUBLIC ${STM32F10X_STDPERIPH_DEFINES})

target_compile_options(blinky.elf PUBLIC "-O0")
set_target_properties(blinky.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")
add_extra_commands(blinky.elf)
