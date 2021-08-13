/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#pragma once

#define bswap16(_x)     __builtin_bswap16(_x)
#define bswap32(_x)     __builtin_bswap32(_x)
#define bswap64(_x)     __builtin_bswap64(_x)

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define be16(_x)    bswap16(_x)
#define be32(_x)    bswap32(_x)
#define be64(_x)    bswap64(_x)

#define le16(_x)    (_x)
#define le32(_x)    (_x)
#define le64(_x)    (_x)

#else

#define be16(_x)    (_x)
#define be32(_x)    (_x)
#define be64(_x)    (_x)

#define le16(_x)    bswap16(_x)
#define le32(_x)    bswap32(_x)
#define le64(_x)    bswap64(_x)

#endif
