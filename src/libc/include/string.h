/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2020 Fernando Lugo <lugo.fernando@gmail.com>.
 */

#pragma once

#include <limits.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

size_t strlen(const char* s);
size_t strnlen(const char* s, size_t maxlen);
char* strchr(const char* s, int c);
char* strrchr(const char* s, int c);

void* memset(void* s, int c, size_t n);
void* memcpy(void* dest, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
void* memmove(void* dest, const void* src, size_t n);
void* memchr(const void* src, int c, size_t n);

/**
 * strtoull - convert a string to an unsigned long long integer
 *
 * The strtoull() function converts the initial part of the string in nptr to an unsigned long long
 * int value according to the given base, which must be 8 10 or 16, or be the special value 0.
 *
 * The string may begin with an arbitrary amount of white spaces followed by a single optional '+'
 * or '-' sign. If base is zero or 16, the string may then include a "0x" prefix, and the number
 * will be read in base 16; otherwise, a zero base is taken as 10 (decimal) unless the next
 * character is '0', in which case it is taken  as  8 (octal)
 *
 * If endptr is not NULL, strtoull() stores the address of the first invalid character in *endptr.
 * If there were no digits at all, strtoull() stores the original value of nptr in *endptr
 * (and returns 0).
 *
 * Since strtoull() can legitimately return 0 or ULLONG_MAX on both success and failure, the
 * calling program should use @endptr to determine if an error occurred by checking whether:
 * - Invalid parameters     If returns 0 and @endptr == @nptr
 * - Overflow               If returns ULLONG_MAX and @endptr == @nptr
 *
 * @nptr        Pointer to the string
 * @endptr      The address of the first invalid will be stored at @endptr
 * @base        Base of the integer to be converted
 *
 * @return      either the result of the conversion  or, if there was a leading minus sign, the
 *              negation of the result of the conversion represented as an unsigned value, unless
 *              the original (nonnegated) value would overflow; in the latter case, strtoull()
 *              returns ULLONG_MAX and sets @endptr to @nptr.
 *
 */
unsigned long long int strtoull(const char* nptr, char** endptr, int base);

/**
 * strtoul - convert a string to an unsigned long integer
 *
 * The strtoul() function converts the initial part of the string in nptr to an unsigned long long
 * int value according to the given base, which must be 8 10 or 16, or be the special value 0.
 *
 * The string may begin with an arbitrary amount of white spaces followed by a single optional '+'
 * or '-' sign. If base is zero or 16, the string may then include a "0x" prefix, and the number
 * will be read in base 16; otherwise, a zero base is taken as 10 (decimal) unless the next
 * character is '0', in which case it is taken  as  8 (octal)
 *
 * If endptr is not NULL, strtoul() stores the address of the first invalid character in *endptr.
 * If there were no digits at all, strtoul() stores the original value of nptr in *endptr
 * (and returns 0).
 *
 * Since strtoul() can legitimately return 0, ULONG_MAX on both success and failure, the
 * calling program should use @endptr to determine if an error occurred by checking whether:
 * - Invalid parameters     If returns 0 and @endptr == @nptr
 * - Overflow               If returns ULONG_MAX and @endptr == @nptr
 *
 * @nptr        Pointer to the string
 * @endptr      The address of the first invalid will be stored at @endptr
 * @base        Base of the integer to be converted
 *
 * @return      either the result of the conversion  or, if there was a leading minus sign, the
 *              negation of the result of the conversion represented as an unsigned value, unless
 *              the original (nonnegated) value would overflow; in the latter case, strtoul()
 *              returns ULONG_MAX and sets @endptr to @nptr.
 *
 */
unsigned long int strtoul(const char* nptr, char** endptr, int base);

/**
 * strtoll - convert a string to an long long integer
 *
 * The strtoll() function converts the initial part of the string in nptr to an unsigned long long
 * int value according to the given base, which must be 8 10 or 16, or be the special value 0.
 *
 * The string may begin with an arbitrary amount of white spaces followed by a single optional '+'
 * or '-' sign. If base is zero or 16, the string may then include a "0x" prefix, and the number
 * will be read in base 16; otherwise, a zero base is taken as 10 (decimal) unless the next
 * character is '0', in which case it is taken  as  8 (octal)
 *
 * If endptr is not NULL, strtoll() stores the address of the first invalid character in *endptr.
 * If there were no digits at all, strtoll() stores the original value of nptr in *endptr
 * (and returns 0).
 *
 * Since strtoll() can legitimately return 0, LLONG_MAX or LLONG_MIN on both success and failure,
 * the calling program should use @endptr to determine if an error occurred by checking whether:
 * - Invalid parameters     If returns 0 and @endptr == @nptr
 * - Overflow               If returns LLONG_MAX and @endptr == @nptr
 * - Underflow              If returns LLONG_MIN and @endptr == @nptr
 *
 * @nptr        Pointer to the string
 * @endptr      The address of the first invalid will be stored at @endptr
 * @base        Base of the integer to be converted
 *
 * @return      returns the result of the conversion, unless the value would underflow or overflow.
 *              If an underflow occurs, strtoll() returns LLONG_MIN.  If an overflow occurs,
 *              strtoll() returns LLONG_MAX.  In both cases, @endptr is set to @nptr
 *
 */
long long int strtoll(const char* nptr, char** endptr, int base);

/**
 * strtol - convert a string to an long integer
 *
 * The strtol() function converts the initial part of the string in nptr to an unsigned long long
 * int value according to the given base, which must be 8 10 or 16, or be the special value 0.
 *
 * The string may begin with an arbitrary amount of white spaces followed by a single optional '+'
 * or '-' sign. If base is zero or 16, the string may then include a "0x" prefix, and the number
 * will be read in base 16; otherwise, a zero base is taken as 10 (decimal) unless the next
 * character is '0', in which case it is taken  as  8 (octal)
 *
 * If endptr is not NULL, strtol() stores the address of the first invalid character in *endptr.
 * If there were no digits at all, strtol() stores the original value of nptr in *endptr
 * (and returns 0).
 *
 * Since strtol() can legitimately return 0, LONG_MAX or LONG_MIN on both success and failure,
 * the calling program should use @endptr to determine if an error occurred by checking whether:
 * - Invalid parameters     If returns 0 and @endptr == @nptr
 * - Overflow               If returns LONG_MAX and @endptr == @nptr
 * - Underflow              If returns LONG_MIN and @endptr == @nptr
 *
 * @nptr        Pointer to the string
 * @endptr      The address of the first invalid will be stored at @endptr
 * @base        Base of the integer to be converted
 *
 * @return      returns the result of the conversion, unless the value would underflow or overflow.
 *              If an underflow occurs, strtol() returns LONG_MIN.  If an overflow occurs,
 *              strtol() returns LONG_MAX.  In both cases, @endptr is set to @nptr
 *
 */
long int strtol(const char* nptr, char** endptr, int base);

#ifdef __cplusplus
}
#endif
