/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2020 Fernando Lugo <lugo.fernando@gmail.com>
 */

#pragma once

// NOTE: add new defines are they are needed

#define CHAR_BIT 8
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#define WORD_BIT 32

// integer limits
#define USHRT_MAX 65535U
#define SHRT_MAX  32767
#define SHRT_MIN  (-32768)

#define UINT_MAX 4294967295U
#define INT_MAX  2147483647
#define INT_MIN  -2147483648

#ifdef __LP64__
#define ULONG_MAX 18446744073709551615UL
#define LONG_MAX  9223372036854775807L
#define LONG_MIN  (-9223372036854775807L - 1L)
#else
#define ULONG_MAX 4294967295UL
#define LONG_MAX  2147483647L
#define LONG_MIN  -2147483648L
#endif

#define ULLONG_MAX 18446744073709551615ULL
#define LLONG_MAX  9223372036854775807LL
#define LLONG_MIN  (-9223372036854775807LL - 1LL)
