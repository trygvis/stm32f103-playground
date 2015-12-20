#include "debug.h"
#include "printf.h"

#ifdef __cplusplus
extern "C" {
#endif

static void append_char(void *v, char c) {
    *(*((char **) v))++ = c;
}

void dbg_printf(const char *fmt, ...) {
    char msg[100];
    msg[0] = 0;

    va_list va;
    va_start(va, fmt);
    char *a = msg;
    char **b = &a;
    tfp_format(b, append_char, fmt, va);
    va_end(va);
    append_char(b, '\0');

    send_command(SYS_WRITE0, msg);
}

void dbg_putc(void *, char c) {
    char cc = c;
    send_command(SYS_WRITEC, &cc);
}

void send_command(enum SemihostingCmd command, void *message) {
    int c = command;

    __asm volatile (
    "mov r0, %[cmd];"
        "mov r1, %[msg];"
        "bkpt #0xAB" : : [cmd] "r"(c), [msg] "r"(message) : "r0", "r1", "memory"
    );
}

#ifdef __cplusplus
};
#endif
