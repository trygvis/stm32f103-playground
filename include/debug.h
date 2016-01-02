#ifndef DEBUG_H
#define DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((format (printf, 1, 2)))
void dbg_printf(const char *fmt, ...);

void dbg_putc(void * junk, char);

enum SemihostingCmd {
    SYS_CLOSE = 0x02,
    SYS_CLOCK = 0x10,
    SYS_ELAPSED = 0x30,
    SYS_ERRNO = 0x13,
    SYS_FLEN = 0x0C,
    SYS_GET_CMDLINE = 0x15,
    SYS_HEAPINFO = 0x16,
    SYS_ISERROR = 0x08,
    SYS_ISTTY = 0x09,
    SYS_OPEN = 0x01,
    SYS_READ = 0x06,
    SYS_READC = 0x07,
    SYS_REMOVE = 0x0E,
    SYS_RENAME = 0x0F,
    SYS_SEEK = 0x0A,
    SYS_SYSTEM = 0x12,
    SYS_TICKFREQ = 0x31,
    SYS_TIME = 0x11,
    SYS_TMPNAM = 0x0D,
    SYS_WRITE = 0x05,
    SYS_WRITEC = 0x03,
    SYS_WRITE0 = 0x04,
};

void send_command(enum SemihostingCmd command, void *message);

#ifdef __cplusplus
};
#endif

#endif
