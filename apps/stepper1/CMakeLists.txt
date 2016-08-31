add_executable(stepper1.elf stepper1.cpp
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/CoreSupport/core_cm3.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/misc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
        ${STM32F10X_STDPERIPH_LIB}/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.c
        $<TARGET_OBJECTS:playground>
        )

target_include_directories(stepper1.elf PUBLIC
        $<TARGET_PROPERTY:playground,INTERFACE_INCLUDE_DIRECTORIES>
        )
target_compile_definitions(stepper1.elf PUBLIC ${STM32F10X_STDPERIPH_DEFINES})
target_link_libraries(stepper1.elf tinyprintf)

set_target_properties(stepper1.elf PROPERTIES LINK_FLAGS "-nostartfiles -T${CMAKE_SOURCE_DIR}/cmake/stm32.ld")
add_extra_commands(stepper1.elf)
