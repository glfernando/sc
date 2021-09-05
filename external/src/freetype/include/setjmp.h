
#pragma once

typedef long jmp_buf[32];

#define setjmp(x)   (0)
#define longjmp(x, y) do { \
    (void)x; \
    (void)y; \
} while(0)

