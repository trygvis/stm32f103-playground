add_executable(max7219.elf max7219.cpp alphabet.h
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/misc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c
        $<TARGET_OBJECTS:playground>
        )

target_include_directories(max7219.elf PUBLIC
        $<TARGET_PROPERTY:playground,INTERFACE_INCLUDE_DIRECTORIES>
        )
target_compile_definitions(max7219.elf PUBLIC ${STM32F10X_STDPERIPH_DEFINES})
target_link_libraries(max7219.elf tinyprintf m)

set_target_properties(max7219.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")
add_extra_commands(max7219.elf)
