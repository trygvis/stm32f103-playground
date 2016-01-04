#include <stdint.h>
#include <stm32f10x.h>

#include "debug.h"
#include "tinyprintf.h"
#include "playground.h"

extern "C"
__attribute__((naked, used))
void HardFault_Handler_C(uint32_t *hardfault_args) {
    dbg_printf("r0  = 0x%08lx (%lu)\n", hardfault_args[0], hardfault_args[0]);
    dbg_printf("r1  = 0x%08lx (%lu)\n", hardfault_args[1], hardfault_args[1]);
    dbg_printf("r2  = 0x%08lx (%lu)\n", hardfault_args[2], hardfault_args[2]);
    dbg_printf("r3  = 0x%08lx (%lu)\n", hardfault_args[3], hardfault_args[3]);
    dbg_printf("r12 = 0x%08lx (%lu)\n", hardfault_args[4], hardfault_args[4]);
    dbg_printf("lr  = 0x%08lx (%lu)\n", hardfault_args[5], hardfault_args[5]);
    dbg_printf("pc  = 0x%08lx (%lu)\n", hardfault_args[6], hardfault_args[6]);
    dbg_printf("psr = 0x%08lx (%lu)\n", hardfault_args[7], hardfault_args[7]);
    dbg_printf("\n");

    halt();
}

volatile bool run = true;

class StaticMyClass {
public:
    StaticMyClass(int value) : value(value) {
        dbg_printf("StaticMyClass::StaticMyClass(%d)\n", value);
    }

    // Destructors are not supported for classes that are globally allocated.
//    ~StaticMyClass() {
//    }

    int value;
};

class ConstStaticMyClass {
public:
    ConstStaticMyClass(int value) : value(value) {
        dbg_printf("ConstStaticMyClass::ConstStaticMyClass(%d)\n", value);
    }

    int value;
};

class MyClass {
public:
    MyClass(int value) : value(value) {
        dbg_printf("MyClass::MyClass(%d)\n", value);
    }

    ~MyClass() {
        dbg_printf("MyClass::~MyClass\n");
    }

    int value;
};

StaticMyClass staticInstance(1337);
static const ConstStaticMyClass constStaticInstance(9876);

/*
 * When we get there the stack pointer is set
 */
int main() {
    SystemInit();

    init_printf(nullptr, dbg_putc);

    dbg_printf("C++ Test #1\n");

    dbg_printf("staticInstance.value=%d\n", staticInstance.value);
    dbg_printf("constStaticInstance.value=%d\n", constStaticInstance.value);

    {
        MyClass instance2(1234);
        dbg_printf("instance2.value=%d\n", instance2.value);
    }

    dbg_printf("Sleeping..\n");
    while (run) {
        __NOP();
    }

    return 0;
}
