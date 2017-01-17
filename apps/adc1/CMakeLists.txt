add_executable(adc1.elf adc1.cpp
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/misc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c
        $<TARGET_OBJECTS:playground>
        )

target_include_directories(adc1.elf PUBLIC
        $<TARGET_PROPERTY:playground,INTERFACE_INCLUDE_DIRECTORIES>
        )
target_compile_definitions(adc1.elf PUBLIC ${STM32F10X_STDPERIPH_DEFINES})
target_link_libraries(adc1.elf tinyprintf m)

set_target_properties(adc1.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")
add_extra_commands(adc1.elf)
