/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2020 Fernando Lugo <lugo.fernando@gmail.com>.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Formatting flags
 */
#define LEFT    (1 << 0)
#define ZEROPAD (1 << 1)
#define SIGN    (1 << 2)
#define CAPS    (1 << 3)
#define POINTER (1 << 4)

/* 22 string fits 64 bits integer */
#define NUM_BUF_MAX 22

/*
 * Lets allow someone to overload putchar for a very early printf support
 */
__attribute__((weak)) int putchar(int c) {
    return c;
}

static int (*custom_putchar)(int c);

static int print_char(char** pbuf, char* end, char c) {
    if (pbuf) {
        if (*pbuf != end)
            *(*pbuf)++ = c;
        else
            return 0;
    } else {
        custom_putchar ? custom_putchar(c) : putchar(c);
    }
    return 1;
}

static int print_str(char** pbuf, char* end, char* s, int w, int f) {
    char pad = f & ZEROPAD ? '0' : ' ';
    int len, i, n;
    char* t;

    /* calculate lenght of string */
    for (t = s; *t; ++t) {}

    len = t - s;

    if (f & POINTER)
        len += 2;

    n = 0;

    /* if zero pad add prefix before padding */
    if ((f & POINTER) && (f & ZEROPAD)) {
        n += print_char(pbuf, end, '0');
        n += print_char(pbuf, end, 'x');
        len -= 2;
    }

    if (!(f & LEFT))
        while (len < w--)
            n += print_char(pbuf, end, pad);

    /* if not zero pad add prefix after padding */
    if ((f & POINTER) && !(f & ZEROPAD)) {
        n += print_char(pbuf, end, '0');
        n += print_char(pbuf, end, 'x');
        len -= 2;
    }

    for (i = 0; i < len; i++)
        n += print_char(pbuf, end, *s++);

    while (len < w--)
        n += print_char(pbuf, end, pad);

    return n;
}

static int print_num(char** pbuf, char* end, uint64_t n, int b, int w, int f) {
    char num_buf[NUM_BUF_MAX];
    int t, neg = 0, pc = 0;
    unsigned mask = 0, shift = 0;
    int l = f & CAPS ? 'A' : 'a';
    char* s;

    if (n == 0ULL) {
        num_buf[0] = '0';
        num_buf[1] = '\0';
        return print_str(pbuf, end, num_buf, w, f);
    }

    /* check if negative value for base 10 number */
    if (b == 10 && f & SIGN && (int64_t)n < 0) {
        neg = 1;
        n = -(int64_t)n;
    }

    s = num_buf + NUM_BUF_MAX - 1;
    *s = '\0';

    /*
     * mask and shift for base 8 and 16 (to avoid division)
     */
    mask = b - 1;
    shift = 3 + (b >> 4);

    while (n) {
        if (b == 10) {
            t = n % b;
            n /= b;
        } else {
            t = n & mask;
            n >>= shift;
        }

        if (t > 9)
            t += l - '0' - 10;
        *--s = t + '0';
    }

    if (neg) {
        if (w && (f & ZEROPAD)) {
            pc += print_char(pbuf, end, '-');
            --w;
        } else {
            *--s = '-';
        }
    }

    return pc + print_str(pbuf, end, s, w, f);
}

int vsnprintf(char* buf, size_t size, const char* fmt, va_list va) {
    int n = 0;
    char** pbuf = buf ? &buf : NULL;
    char* end = buf + size;

    if (!size)
        return 0;

    /* leave space for '\0' */
    end--;

    for (; *fmt; ++fmt) {
        uint64_t num;
        int w, q, f, b;
        /* check for simple character */
        if (*fmt != '%') {
            n += print_char(pbuf, end, *fmt);
            continue;
        }

        fmt++;
        /* if no more characters or another % just printf % */
        if (!*fmt || *fmt == '%') {
            n += print_char(pbuf, end, '%');
            continue;
        }

        f = 0;
        if (*fmt == '-') {
            if (!*++fmt)
                break;
            f |= LEFT;
        }

        if (*fmt == '0') {
            if (!*++fmt)
                break;
            f |= ZEROPAD;
        }

        w = 0;
        while (*fmt && *fmt >= '0' && *fmt <= '9')
            w = w * 10 + *fmt++ - '0';

        if (!*fmt)
            break;

        q = 0;
        if (*fmt == 'l' || *fmt == 'L') {
            q = *fmt++;
            if (*fmt && *fmt == 'l') {
                if (!*++fmt)
                    break;
                q = 'L';
            }
        }
        if (*fmt == 'z') {
            fmt++;
            if (sizeof(size_t) == 8)
                q = 'l';
        }

        /* default base = 10 */
        b = 10;
        switch (*fmt) {
            char c[2];
            char* s;
        case 'c':
            c[0] = (char)va_arg(va, int);
            c[1] = '\0';
            n += print_str(pbuf, end, c, w, f);
            continue;
        case 's':
            s = va_arg(va, char*);
            n += print_str(pbuf, end, s ?: "(null)", w, f);
            continue;
        case 'o':
            b = 8;
            break;
        case 'X':
            f |= CAPS;
        case 'x':
            b = 16;
            break;
        case 'P':
            f |= CAPS;
        case 'p':
            b = 16;
            f |= POINTER;
            q = 'l';
            break;
        case 'd':
        case 'i':
            f |= SIGN;
        case 'u':
            break;
        default:
            continue;
        }

        if (q == 'L') {
            num = va_arg(va, unsigned long long);
        } else if (q == 'l') {
            num = va_arg(va, unsigned long);
            if (f & SIGN)
                num = (long)num;
        } else {
            num = va_arg(va, unsigned int);
            if (f & SIGN)
                num = (int)num;
        }

        n += print_num(pbuf, end, num, b, w, f);
    }

    if (buf)
        **pbuf = '\0';

    return n;
}

int printf(const char* fmt, ...) {
    va_list args;
    int n;

    va_start(args, fmt);
    n = vsnprintf(NULL, (size_t)-1, fmt, args);
    va_end(args);

    return n;
}

int sprintf(char* buf, const char* fmt, ...) {
    va_list args;
    int n;

    va_start(args, fmt);
    n = vsnprintf(buf, (size_t)-1, fmt, args);
    va_end(args);

    return n;
}

int snprintf(char* buf, size_t size, const char* fmt, ...) {
    va_list args;
    int n;

    va_start(args, fmt);
    n = vsnprintf(buf, size, fmt, args);
    va_end(args);

    return n;
}

/*
 * Allow users to provide their own putchar method. Mostly will be used once a
 * console driver us running
 *
 * @func    Custom putchar function pointer
 *
 */
void printf_set_putchar_func(int (*func)(int c)) {
    custom_putchar = func;
}
