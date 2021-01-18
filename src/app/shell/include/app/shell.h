/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#pragma once

// struct describing a shell command
struct shell_cmd {
    char const* name;
    int (*cmd)(int argc, char const* argv[]);
    char const* desc;
    void (*usage)(void);
};

// helper macro to declare a static shell command
#define shell_declare_static_cmd(name_, desc_, cmd_, usage_)                           \
    struct shell_cmd shell_cmd_##name_ __attribute__((section(".shell_cmds." #name_))) \
        __attribute__((used)) = {                                                      \
            .name = #name_,                                                            \
            .cmd = cmd_,                                                               \
            .desc = desc_,                                                             \
            .usage = usage_,                                                           \
    }

extern struct shell_cmd __shell_cmds_start[];
extern struct shell_cmd __shell_cmds_end[];

void shell_run();
int shell_exec_cmd(char const* str);
