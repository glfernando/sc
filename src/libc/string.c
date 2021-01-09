/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2020 Fernando Lugo <lugo.fernando@gmail.com>.
 */

//#include <limits.h>
#include <stdint.h>
#include <string.h>

#define __MASK(type, _a)    (((typeof(type))(_a)) - 1)
#define ALIGN_DOWN(_x, _a)  ((_x) & ~((_a)-1))
#define ALIGN_UP(_x, _a)    (((_x) + __MASK(_x, _a)) & ~__MASK(_x, _a))
#define ALIGN               ALIGN_UP
#define ALIGNP(_p, _a)      ((void*)ALIGN((unsigned long)(_p), _a))
#define ALIGNP_DOWN(_p, _a) ((void*)ALIGN_DOWN((unsigned long)(_p), _a))

char* strcpy(char* dest, const char* src) {
    char* d = dest;

    while ((*dest++ = *src++)) {}

    return d;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* d = dest;

    while (n-- && (*dest++ = *src++)) {}

    return d;
}

int strcmp(const char* s1, const char* s2) {
    for (; *s1 && *s1 == *s2; ++s1, ++s2) {}

    return *s1 - *s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    if (!n)
        return 0;

    for (; --n && *s1 && *s1 == *s2; ++s1, ++s2) {}

    return *s1 - *s2;
}

size_t strlen(const char* s) {
    const char* t = s;

    while (*t)
        t++;
    return t - s;
}

size_t strnlen(const char* s, size_t maxlen) {
    const char* t = s;

    while (maxlen-- && *t)
        t++;
    return t - s;
}

char* strchr(const char* s, int c) {
    for (; *s; ++s)
        if (*s == c)
            return (char*)s;

    return NULL;
}

char* strrchr(const char* s, int c) {
    char* p = NULL;

    for (; *s; ++s)
        if (*s == c)
            p = (char*)s;

    return p;
}

void* memset(void* s, int c, size_t n) {
    uint8_t* p1 = s;
    uint8_t* p2;
    uint64_t v;

    if (n < 8)
        goto last;

    /* align to 8 bytes */
    p2 = ALIGNP(s, 8);
    n -= p2 - p1;

    while (p1 < p2)
        *p1++ = c;

    if (!c) {
        v = 0;
    } else {
        v = c;
        v |= v << 8;
        v |= v << 16;
        v |= v << 32;
    }

    for (; n >= 8; n -= 8, p1 += 8)
        *(uint64_t*)p1 = v;

last:
    while (n--)
        *p1++ = c;

    return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = dest;
    const uint8_t* s = src;
    uint8_t* p;

    if (n < 8)
        goto last;

    /* check if both can be aligned to 8 bytes */
    if (((unsigned long)dest & 0x7) != ((unsigned long)src & 0x7))
        goto last;

    p = ALIGNP(d, 8);
    n -= p - d;

    /* copy unaligned data first */
    while (d < p)
        *d++ = *s++;

    /* copy aligned data */
    for (; n >= 8; n -= 8, d += 8, s += 8)
        *(uint64_t*)d = *(uint64_t*)s;

last:
    while (n--)
        *d++ = *s++;

    return dest;
}

int memcmp(const void* src1, const void* src2, size_t n) {
    const uint8_t* s1 = src1;
    const uint8_t* s2 = src2;

    if (!n)
        return 0;

    for (; --n && *s1 == *s2; ++s1, ++s2) {}

    return *s1 - *s2;
}

void* memmove(void* dest, const void* src, size_t n) {
    if (dest == src || !n)
        return dest;

    size_t dist = src > dest ? src - dest : dest - src;
    /* if there is not overlap use memcpy */
    if (dist > n)
        return memcpy(dest, src, n);

    int x;
    uint8_t* d;
    const uint8_t* s;

    if (src > dest) {
        x = 1;
        d = dest;
        s = src;
    } else {
        /* we will copy from the end to the beginning */
        x = -1;
        d = dest + n;
        s = src + n;
    }

    if (n < 8 || dist < 8)
        goto last;

    /* check if both can be aligned to 8 bytes */
    if (((unsigned long)dest & 0x7) != ((unsigned long)src & 0x7))
        goto last;

    uint8_t* p;
    unsigned tmp;
    if (x == 1) {
        p = ALIGNP(d, 8);
        tmp = p - d;
    } else {
        p = ALIGNP_DOWN(d, 8);
        tmp = d - p;
    }

    n -= tmp;
    /* copy unaligned data first */
    d = d - (x == -1);
    s = s - (x == -1);
    for (; tmp--; d += x, s += x)
        *d = *s;
    d = d + (x == -1);
    s = s + (x == -1);

    /* copy aligned data */
    d = d - 8 * (x == -1);
    s = s - 8 * (x == -1);
    for (; n >= 8; n -= 8, d += x * 8, s += x * 8)
        *(uint64_t*)d = *(uint64_t*)s;
    d = d + 8 * (x == -1);
    s = s + 8 * (x == -1);
last:
    d = d - (x == -1);
    s = s - (x == -1);
    for (; n--; d += x, s += x)
        *d = *s;

    return dest;
}

void* memchr(const void* src, int c, size_t n) {
    unsigned char* s = (unsigned char*)src;

    for (; n--; s++)
        if (*s == c)
            return s;

    return NULL;
}

int internal_strtoull(const char* nptr, char** endptr, int base, int* neg, unsigned long long* v) {
    if (endptr)
        *endptr = (char*)nptr;

    *v = 0;
    *neg = 0;

    /* skip all spaces */
    while (*nptr == ' ')
        nptr++;

    /* check sign */
    if (*nptr == '+') {
        nptr++;
    } else if (*nptr == '-') {
        *neg = 1;
        nptr++;
    }

    if (!*nptr)
        return 0;

    /* we only support bases 0, 8, 10 and 16 */
    switch (base) {
    case 0:
        /* calculate base */
        if (*nptr == '0') {
            if ((*(++nptr) | 0x20) == 'x') {
                base = 16;
                nptr++;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
        break;
    case 8:
    case 16:
        if (*nptr == '0')
            nptr++;
        if (base == 16 && (*nptr | 0x20) == 'x')
            nptr++;
    case 10:
        break;
    default:
        return 0;
    }

    char c;
    for (; (c = *nptr); ++nptr) {
        int x;
        if (c >= '0' && c <= '9')
            x = c - '0';
        else if ((c |= 0x20) >= 'a' && c <= 'f')  // conver to lower case and check
            x = c - 'a' + 10;
        else
            break;
        /* check if it is valid for current base */
        if (x >= base)
            break;
        /* v = v * base + x; */
        if (__builtin_umulll_overflow(*v, base, v) || __builtin_uaddll_overflow(*v, x, v))
            return 1;  // overflow
    }

    if (endptr)
        *endptr = (char*)nptr;

    return 0;
}

unsigned long long strtoull(const char* nptr, char** endptr, int base) {
    unsigned long long v;
    int neg;

    if (internal_strtoull(nptr, endptr, base, &neg, &v))
        goto overflow;

    if (neg && v > ULLONG_MAX / 2 + 1)
        goto overflow;
    return neg ? -v : v;

overflow:
    if (endptr)
        *endptr = (char*)nptr;
    return ULLONG_MAX;
}

long long strtoll(const char* nptr, char** endptr, int base) {
    unsigned long long v;
    int neg;

    if (internal_strtoull(nptr, endptr, base, &neg, &v))
        goto overflow;

    /* check for overflow */
    if (v > ULLONG_MAX / 2 + neg)
        goto overflow;

    return neg ? -v : v;

overflow:
    if (endptr)
        *endptr = (char*)nptr;
    return neg ? LLONG_MIN : LLONG_MAX;
}

unsigned long strtoul(const char* nptr, char** endptr, int base) {
    unsigned long long v;
    int neg;

    if (internal_strtoull(nptr, endptr, base, &neg, &v))
        goto overflow;

    if (neg && v > ULONG_MAX / 2 + 1)
        goto overflow;
    return neg ? -v : v;

overflow:
    if (endptr)
        *endptr = (char*)nptr;
    return ULONG_MAX;
}

long strtol(const char* nptr, char** endptr, int base) {
    unsigned long long v;
    int neg;

    if (internal_strtoull(nptr, endptr, base, &neg, &v))
        goto overflow;

    /* check for overflow */
    if (v > ULONG_MAX / 2 + neg)
        goto overflow;

    return neg ? -v : v;

overflow:
    if (endptr)
        *endptr = (char*)nptr;
    return neg ? LONG_MIN : LONG_MAX;
}
