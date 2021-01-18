/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * (C) Copyright 2020, Fernando Lugo <lugo.fernando@gmail.com>
 */

#pragma once

#define CONFIG_TEXT_BASE       0x00080000
#define CONFIG_STACK_SIZE      (1024 * 1024)
#define CONFIG_HEAP_SIZE       (1024 * 1024 * 10)
#define CONFIG_DEBUG_UART_BASE 0xfe201000
