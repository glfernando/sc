/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

/*
 * SCSH is a minimal shell with essential features such as history, line editing, and command auto
 * complete. However, it uses builtin types, the reason for not using std::string or other
 * facilities such as std::vector is that we should not rely on dynamic allocation. The intention is
 * to allow to execute shell in crash scenarios for debugging purposes, if the crash was because of
 * memory lack, that could be an issue if the shell uses dynamic memory
 */

#include <app/shell.h>
#include <errcodes.h>
#include <string.h>

import lib.fmt;
import board.peripherals;
import lib.exception;

using lib::exception;
using lib::fmt::print;
using lib::fmt::println;

constexpr unsigned HISTORY_SIZE = 32;  // needs to be at least 1
constexpr unsigned MAX_LINE_SIZE = 128;

constexpr char PROMPT[] = "scsh>";
constexpr auto PROMPT_SIZE = sizeof PROMPT;

// clang-format off
// some special chars
constexpr char BACKSPACE    = '\b';
constexpr char DEL          = '\x7f';
constexpr char TAB          = '\t';
constexpr char BELL         = '\a';
constexpr char ESC          = '\e';

// cursor movements
constexpr auto CUR_BACK     = "\e[D";
constexpr auto CUR_FWD      = "\e[C";

// erase cores
constexpr auto ERASE_EOL    = "\e[K";
// clang-format off

static char history[HISTORY_SIZE][MAX_LINE_SIZE];
static unsigned hi;

static inline int getc(void)
{
    auto& con = board::peripherals::default_console();
    return con.getc();
}

static inline void putc(int c)
{
    auto& con = board::peripherals::default_console();
    return con.putc(c);
}

static inline void puts(char const* str)
{
    auto& con = board::peripherals::default_console();
    while (int c = *str++)
        con.putc(c);
}

// helpe class to iterate over commands
class shell_commands {
 public:
    using it = shell_cmd*;
    shell_commands(it start, it end) : start(start), end_(end) {}
    it begin() { return start; }
    it end() { return end_; }
    shell_cmd& operator[](int index) { return start[index]; }

 private:
    it start;
    it end_;
};

// create shell_commands object with all static commands
static shell_commands commands(__shell_cmds_start, __shell_cmds_end);

static void shell_help_list_all_cmds(void)
{
    for (auto& cmd : commands)
        println("{:<16} - {}", cmd.name, cmd.desc);
}

static int shell_find_matching_commands(const char* cmd_name, int len, int* index)
{
    // if it has spaces just return
    // TODO: implment autocomplete for command parameters

    for (int i = 0; i < len; ++i)
        if (cmd_name[i] == ' ')
            return 0;

    int c = 0;
    for (auto& cmd : commands) {
        if (!strncmp(cmd.name, cmd_name, len)) {
            if (c++ == 0)
                *index = &cmd - commands.begin();  // set start index
        } else {
            if (c != 0)  // end of the match
                break;
        }
    }

    return c;
}

// return max common part among commands. e.g foo1, foo2, foobar, max common index is 2
static int shell_cmd_max_common(int start, int count)
{
    // sanity check
    if (count == 0)
        return 0;
    if (count == 1)
        return strlen(commands[start].name);

    int i = 0;
    const char* name = commands[start].name;
    shell_cmd* cmd_end = &commands[start + count];
    while (name[i]) {
        for (shell_cmd* cmd = &commands[start + 1]; cmd < cmd_end; ++cmd) {
            if (name[i] != cmd->name[i])
                return i;
        }
        i++;
    }

    return i;
}

// escape sequences we handle
enum ESC_SEQ {
    ESC_NONE,
    ESC_CUR_UP,
    ESC_CUR_DOWN,
    ESC_CUR_FORWARD,
    ESC_CUR_BACKWARD,
    ESC_CUR_HOME,
    ESC_CUR_END,
    ESC_DEL,
};

static inline void set_cursor_pos(int pos)
{
    /* move cursor to 0 */
    putc('\r');
    print("\e[{}C", PROMPT_SIZE + pos);
}

// Parse escape sequences
static enum ESC_SEQ get_esc_seq(void)
{
    /* here we already got the 'ESC' char, so lets get the next characters */
    switch (getc()) {
    case '[':
        switch (getc()) {
        case 'A':
            return ESC_CUR_UP;
        case 'B':
            return ESC_CUR_DOWN;
        case 'C':
            return ESC_CUR_FORWARD;
        case 'D':
            return ESC_CUR_BACKWARD;
        case 'H':
            return ESC_CUR_HOME;
        case 'F':
            return ESC_CUR_END;
        case '1':
            switch (getc()) {
            case '~':
                return ESC_CUR_HOME;
            }
            break;
        case '3':
            switch (getc()) {
            case '~':
                return ESC_DEL;
            }
            break;
        }
        break;
    case 'O':
        switch (getc()) {
        case 'F':
            return ESC_CUR_END;
        }
        break;
    }
    return ESC_NONE;
}

static char* get_line(char* line)
{
    char line_bak[MAX_LINE_SIZE];
    int i;
    int pos = 0;
    int size = 0;
    unsigned h = hi;
    line[0] = '\0';
    int tabs = 0;
    while (true) {
        int c = getc();
        tabs = c == TAB ? tabs + 1 : 0;
        switch (c) {
        case '\r':
        case '\n':
            puts("\r\n");
            line[size] = '\0';
            return line;
        case BACKSPACE:
        case DEL:
            if (pos == 0) {
                putc(BELL);
                break;
            }
            puts(CUR_BACK);
            pos--;
            size--;
            for (i = pos; i < size; ++i) {
                int c = line[i + 1];
                line[i] = c;
                putc(c);
            }
            puts(ERASE_EOL);
            set_cursor_pos(pos);
            break;
        case ESC:
            switch (get_esc_seq()) {
            case ESC_CUR_UP:
                if (h == 0 || (hi - h) >= HISTORY_SIZE) {
                    putc(BELL);
                    break;
                }
                if (hi == h--) {
                    line[size] = '\0';
                    strcpy(line_bak, line);
                }
                strcpy(line, history[h % HISTORY_SIZE]);
                set_cursor_pos(0);
                for (i = 0; line[i]; ++i)
                    putc(line[i]);
                pos = size = i;
                puts(ERASE_EOL);
                break;
            case ESC_CUR_DOWN:
                if (hi == h) {
                    putc(BELL);
                    break;
                }
                h++;
                if (hi == h)
                    strcpy(line, line_bak);
                else
                    strcpy(line, history[h % HISTORY_SIZE]);
                set_cursor_pos(0);
                for (i = 0; line[i]; ++i)
                    putc(line[i]);
                pos = size = i;
                puts(ERASE_EOL);
                break;
            case ESC_CUR_FORWARD:
                if (pos == size) {
                    putc(BELL);
                    break;
                }
                puts(CUR_FWD);
                pos++;
                break;
            case ESC_CUR_BACKWARD:
                if (pos == 0) {
                    putc(BELL);
                    break;
                }
                puts(CUR_BACK);
                pos--;
                break;
            case ESC_CUR_HOME:
                pos = 0;
                set_cursor_pos(0);
                break;
            case ESC_CUR_END:
                pos = size;
                set_cursor_pos(size);
                break;
            case ESC_DEL:
                if (pos == size) {
                    putc(BELL);
                    break;
                }
                size--;
                for (i = pos; i < size; ++i) {
                    int c = line[i + 1];
                    line[i] = c;
                    putc(c);
                }
                puts(ERASE_EOL);
                set_cursor_pos(pos);
                break;
            default:
                break;
            }
            break;
        case TAB:
            if (pos != size) {
                // TODO: add support for autocomplete when need to insert in the current line
                putc(BELL);
                break;
            }

            if (pos) {
                int index = 0;
                int count = shell_find_matching_commands(line, pos, &index);
                if (count == 0) {
                    putc(BELL);
                    break;
                }

                // lets find max common part of the command;
                int mci = shell_cmd_max_common(index, count);
                if (pos < mci) {
                    // update command to cover all common part

                    // get renaming of the command
                    const char* rem = &commands[index].name[pos];
                    while (pos < mci) {
                        line[pos++] = *rem;
                        size++;
                        putc(*rem++);
                    }
                } else if (pos == mci && count > 1) {
                    // print all commands and restore prompt
                    putc('\n');
                    for (int i = 0; i < count; ++i) {
                        const char* name = commands[index + i].name;
                        // TODO: change to horizontal printing once we have lots of commands
                        println("{}", name);
                    }
                    // restore prompt
                    puts(PROMPT);
                    putc(' ');
                    for (int i = 0; i < pos; ++i)
                        putc(line[i]);
                }

                if (count == 1) {
                    // add space
                    line[pos++] = ' ';
                    putc(' ');
                    size++;
                }
            } else {
                // nothing to autocomplete
                putc(BELL);
            }
            break;
        default:
            /* check if we cannot accept more chars */
            if (size == MAX_LINE_SIZE - 1) {
                putc(BELL);
                break;
            }
            if (pos == size) {
                putc(c);
                line[pos++] = c;
                size++;
                break;
            }
            /* we are inserting chars so move other data */
            for (i = size; i > pos; --i)
                line[i] = line[i - 1];
            putc(c);
            line[pos++] = c;
            size++;
            for (i = pos; i < size; ++i)
                putc(line[i]);
            set_cursor_pos(pos);
        };
    }

    return line;
}

static int shell_help(int argc, char const* argv[])
{
    if (argc == 1) {
        shell_help_list_all_cmds();
        return 0;
    }

    for (auto& cmd : commands) {
        if (!strcmp(cmd.name, argv[1]) && cmd.usage) {
            cmd.usage();
            return 0;
        }
    }

    println("help for cmd '{}' not found", argv[1]);
    return ERR_NOT_FOUND;
}

static int shell_run_cmd(int argc, char const* argv[])
{
    for (auto& cmd : commands) {
        if (!strcmp(cmd.name, argv[0]))
            return cmd.cmd(argc, argv);
    }

    println("command '{}' not found", argv[0]);
    return ERR_NOT_FOUND;
}

static char data[MAX_LINE_SIZE];
static char* argv[MAX_LINE_SIZE / 2];

int shell_exec_cmd(char const* str) {
    char *line = data;
    strncpy(line, str, MAX_LINE_SIZE);
    line[MAX_LINE_SIZE - 1] = '\0';

    // remove all leading spaces
    while (*line == ' ')
        line++;

    if (!*line)
        return ERR_NOT_FOUND;

    int argc = 0;
    // get number of args
    do {
        argv[argc++] = line;
        while (*line && *line != ' ')
            line++;
        while (*line == ' ')
            *line++ = '\0';

    } while (*line);

    return shell_run_cmd(argc, (char const**)argv);
}

void shell_run()
{
    while (true) {
        char* line = data;
        puts(PROMPT);
        putc(' ');
        get_line(line);

        // remove all leading spaces
        while (*line == ' ')
            line++;

        if (!*line)
            continue;

        // save command (if different from previous one)
        if (!hi || strcmp(history[(hi - 1) % HISTORY_SIZE], line))
            strcpy(history[(hi++) % HISTORY_SIZE], line);

        //
        // create arguments
        //

        int argc = 0;
        // get number of args
        do {
            argv[argc++] = line;
            while (*line && *line != ' ')
                line++;
            while (*line == ' ')
                *line++ = '\0';

        } while (*line);

        if (!strcmp(argv[0], "exit"))
            break;

        try {
            shell_run_cmd(argc, (char const**)argv);
        } catch (exception& e) {
            println("exception '{}' running command '{}'", e.msg(), argv[0]);
        } catch (...) {
            println("unknown exception running command '{}'", argv[0]);
        }
    }
}

shell_declare_static_cmd(help, "shows command help or list all commands", shell_help, nullptr);
