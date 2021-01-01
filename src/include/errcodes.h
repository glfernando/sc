/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * (C) Copyright 2020, Fernando Lugo <lugo.fernando@gmail.com>
 */

#pragma once

// use #define so it can be included in assembly files too
// add more when needed

#define ERR_GENERIC         -1
#define ERR_INVALID_ARGS    -2
#define ERR_INVALID         -3
#define ERR_FAULT           -4
#define ERR_NOT_FOUND       -5
#define ERR_NO_MEMORY       -6
#define ERR_NOT_READY       -7
#define ERR_TIMED_OUT       -8
#define ERR_DATA_CORRUPTED  -9
#define ERR_TOO_BIG         -10
#define ERR_TOO_SMALL       -11
#define ERR_OUT_OF_BOUNDS   -12
#define ERR_NOT_IMPLEMENTED -13
#define ERR_NOT_ALLOWED     -14
#define ERR_SEC_VIOLATION   -15
#define ERR_INVALID_STATE   -16
#define ERR_CHANNEL_CLOSED  -17
#define ERR_VERIFICATION    -18
#define ERR_AUTHENTICATION  -19
