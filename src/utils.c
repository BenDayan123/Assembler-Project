#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include "../headers/error.h"
#include "../headers/globals.h"

/*
 * The actual dictionary implementation.
 * -> NOTE: THE ORDER MUST MATCH THE 'opcode_name' ENUM IN 'globals.h' FILE! <-
 */
const CmdInfo operations[] = {
    /* { "name", opcode, funct, op_count } */
    {"mov", 0, 0, 2},
    {"cmp", 1, 0, 2},
    {"add", 2, 10, 2},
    {"sub", 2, 11, 2},
    {"lea", 4, 0, 2},
    {"clr", 5, 10, 1},
    {"not", 5, 11, 1},
    {"inc", 5, 12, 1},
    {"dec", 5, 13, 1},
    {"jmp", 9, 10, 1},
    {"bne", 9, 11, 1},
    {"jsr", 9, 12, 1},
    {"red", 12, 0, 1},
    {"prn", 13, 0, 1},
    {"rts", 14, 0, 0},
    {"stop", 15, 0, 0},
    {NULL, 0, 0, 0}};

CmdInfo *find_cmd_info(char *cmd)
{
    int i;
    for (i = 0; operations[i].name != NULL; i++)
    {
        if (strcmp(cmd, operations[i].name) == 0)
            return &operations[i];
    }
    return NULL;
}

void *handle_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL)
        log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, NULL);
    return ptr;
}

char *create_file_path(const char *path, char *filename, const char *ext)
{
    int path_len = path ? strlen(path) : 0;
    char *new_filename = (char *)malloc(strlen(filename) + strlen(ext) + path_len + 3);
    if (new_filename == NULL)
        return NULL;
    if (path == NULL || *path == '\0')
        sprintf(new_filename, "%s.%s", filename, ext);
    else
        sprintf(new_filename, "%s/%s.%s", path, filename, ext);
    return new_filename;
}
/*
 * Advances the pointer to skip any leading whitespace (spaces, tabs).
 */
char *skip_whitespaces(char *str)
{
    while (*str && isspace(*str))
        str++;
    return str;
}

void get_first_word(char *line, char *word)
{
    int i = 0;
    int j = 0;

    line = skip_whitespaces(line);

    while (line[i] && !isspace(line[i]))
        word[j++] = line[i++];

    word[j] = '\0';
}

int get_next_word(char **src, char *dest, boolean expect_comma)
{
    char *curr = *src;
    int i = 0;
    if (*curr == '\0')
        return -1;
    *dest = '\0';
    curr = skip_whitespaces(curr);

    if (expect_comma)
    {
        if (*curr == ',')
        {
            dest[0] = '\0';
            *src = (++curr);
            return -1;
        }
        while (*curr && *curr != ',')
            dest[i++] = *curr++;
        while (i > 0 && isspace(dest[i - 1]))
            dest[--i] = '\0';
        if (*curr == ',')
            curr++;
    }
    else
    {
        while (*curr && !isspace(*curr))
            dest[i++] = *curr++;

        dest[i] = '\0';
    }
    *src = curr;
    return i - 1;
}

int count_and_validate_data_numbers(char *line)
{
    int count = 0;
    char *ptr = line;
    int expecting_comma = FALSE;

    ptr = skip_whitespaces(ptr);

    if (ptr == NULL || *ptr == '\0')
    {
        log_error(ERR_MISSING_OPERAND, 0, NULL, ".data");
        return 0;
    }

    while (*ptr != '\0')
    {
        if (!expecting_comma)
        {
            char *endptr;
            strtol(ptr, &endptr, 10);

            if (*ptr == ',')
            {
                log_error(ERR_COMMA_AT_START, 0, NULL, ".data");
                return -1;
            }
            if (ptr == endptr)
            {
                log_error(ERR_INVALID_NUMBER, 0, NULL, NULL);
                return -1;
            }
            if (*endptr != '\0' && !isspace(*endptr) && *endptr != ',')
            {
                log_error(ERR_INVALID_NUMBER, 0, NULL, "Float or invalid characters");
                return -1;
            }
            count++;
            ptr = endptr;
            expecting_comma = TRUE;
        }
        else
        {
            ptr = skip_whitespaces(ptr);
            if (*ptr == '\0')
                break;
            if (*ptr == ',')
            {
                ptr++;
                expecting_comma = FALSE;
            }
            else
            {
                log_error(ERR_MISSING_COMMA, 0, NULL, NULL);
                return -1;
            }
        }
    }
    if (!expecting_comma)
    {
        log_error(ERR_COMMA_AT_END, 0, NULL, ".data");
        return -1;
    }
    return count;
}

int is_register(const char *op)
{
    return (strlen(op) == 2 && op[0] == 'r' && op[1] >= '0' && op[1] <= '7');
}

boolean is_vaild_command_line(char *cmd_name, char *args, int line, char *filename)
{
    char *ptr = args;
    char curr_arg[MAX_LINE_LEN] = {0};
    int found_ops = 0, i;
    CmdInfo *cmd = find_cmd_info(cmd_name);

    if (cmd == NULL)
        return log_error(ERR_UNKNOWN_INSTRUCTION, line, filename, cmd_name);

    for (i = 0; i < cmd->op_count; i++)
    {
        curr_arg[0] = '\0';
        get_next_word(&ptr, curr_arg, i < cmd->op_count - 1);
        found_ops += (curr_arg[0] != '\0') ? 1 : 0;
    }
    ptr = skip_whitespaces(ptr);
    if (*ptr != '\0')
        return log_error(ERR_TOO_MANY_OPERANDS, line, filename, cmd_name);
    if (found_ops < cmd->op_count)
        return log_error(ERR_MISSING_OPERAND, line, filename, cmd_name);
    return TRUE;
}

addressing_mode get_arg_mode(char *arg)
{
    if (*arg == '#')
        return IMMEDIATE_ADDR;
    if (*arg == '%')
        return INDEX_ADDR;
    if (is_register(arg))
        return REGISTER_ADDR;
    return DIRECT_ADDR;
}