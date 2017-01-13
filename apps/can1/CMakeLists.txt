add_executable(can1.elf can1.cpp
        ${PLAYGROUND_DIR}/src/init_low.s ${PLAYGROUND_DIR}/src/init_high.cpp ${PLAYGROUND_DIR}/include/init_high.h
        ${PLAYGROUND_DIR}/include/playground.h
        ${PLAYGROUND_DIR}/src/debug.cpp ${PLAYGROUND_DIR}/include/debug.h
        ${PLAYGROUND_DIR}/include/stm32f10x_conf.h
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/misc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_can.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
        )
target_link_libraries(can1.elf tinyprintf)

target_include_directories(can1.elf PUBLIC
        ${PLAYGROUND_DIR}/include
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/inc)

target_compile_definitions(can1.elf PUBLIC
        ${STM32F10X_STDPERIPH_DEFINES})

set_target_properties(can1.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")
add_extra_commands(can1.elf)
