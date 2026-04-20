/*
 * File: utils.c
 * Description: General utility functions for string parsing and memory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include "../headers/error.h"
#include "../headers/globals.h"

/*
 * Dictionary of valid operations.
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
    {NULL, 0, 0, 0}}; /* Sentinel value to mark the end of the array */

/**
 * Function: find_cmd_info
 * Description: Searches for a command in the operations table by name.
 * @param cmd - The command name to search for
 * @return Pointer to the CmdInfo structure if found, NULL otherwise
 */
const CmdInfo *find_cmd_info(char *cmd)
{
    int i;
    /* Iterate through the operations array until we find a match or reach the end */
    for (i = 0; operations[i].name != NULL; i++)
    {
        /* Compare the command name with the current entry */
        if (strcmp(cmd, operations[i].name) == 0)
            return &operations[i]; /* Return pointer to the matching command info */
    }

    /* Command not found in the table */
    return NULL;
}

/**
 * Function: handle_malloc
 * Description: Wrapper for malloc that checks for allocation errors and logs them.
 * @param size - The number of bytes to allocate
 * @return Pointer to the allocated memory, or NULL if allocation failed
 */
void *handle_malloc(size_t size)
{
    /* Attempt to allocate the requested memory */
    void *ptr = malloc(size);
    if (ptr == NULL) /* Check if allocation failed */
        log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, NULL);
    return ptr;
}

/**
 * Function: create_file_path
 * Description: Combines path, filename, and extension into one string.
 * @param path - The directory path (use NULL for current directory)
 * @param filename - The base filename without extension
 * @param ext - The file extension without the dot
 * @return Dynamically allocated string containing the full path, or NULL if allocation failed
 */
char *create_file_path(const char *path, char *filename, const char *ext)
{
    /* Calculate the total length needed for the full path */
    int path_len = path ? strlen(path) : 0;

    /* Allocate memory for the full path string (+3 for '/', '.', and '\0') */
    char *new_filename = (char *)malloc(strlen(filename) + strlen(ext) + path_len + 3);

    if (new_filename == NULL)
        return NULL;

    /* Format the string based on whether a path was provided */
    if (path == NULL || *path == '\0')
        sprintf(new_filename, "%s.%s", filename, ext); /* No path: "filename.ext" */
    else
        sprintf(new_filename, "%s/%s.%s", path, filename, ext); /* With path: "path/filename.ext" */

    return new_filename;
}

/**
 * Function: skip_whitespaces
 * Description: Advances the pointer to skip any leading whitespace characters.
 * @param str - Pointer to the string to process
 * @return Pointer to the first non-whitespace character in the string
 */
char *skip_whitespaces(char *str)
{
    /* Move the pointer forward while the current character is whitespace */
    while (*str && isspace(*str))
        str++;

    return str;
}

/**
 * Function: get_first_word
 * Description: Extracts the first whitespace-delimited word from a line.
 * @param line - The input line to parse
 * @param word - Buffer to store the extracted word
 */
void get_first_word(char *line, char *word)
{
    int i = 0; /* Index for reading from line */
    int j = 0; /* Index for writing to word */

    line = skip_whitespaces(line); /* Skip any leading whitespace */

    /* Copy characters until we hit whitespace or end of string */
    while (line[i] && !isspace(line[i]))
        word[j++] = line[i++];

    word[j] = '\0'; /* Null-terminate the extracted word */
}

/**
 * Function: get_next_word
 * Description: Extracts the next word from the current string pointer and advances the pointer.
 * @param src - Pointer to the string pointer (modified to point after the extracted word)
 * @param dest - Buffer to store the extracted word
 * @param expect_comma - TRUE if a comma separator is expected between words
 * @return The index of the last character in dest, or -1 if no word was found
 */
int get_next_word(char **src, char *dest, boolean expect_comma)
{
    char *curr = *src;
    int i = 0;

    /* Check if we've reached the end of the string */
    if (*curr == '\0')
        return -1;
    *dest = '\0';                  /* Initialize destination to empty string */
    curr = skip_whitespaces(curr); /* Skip leading whitespace */

    /* Handle comma-separated parsing if expected */
    if (expect_comma)
    {
        /* If we encounter a comma immediately, it's an error case */
        if (*curr == ',')
        {
            dest[0] = '\0';
            *src = (++curr);
            return -1;
        }
        /* Extract word until we hit a comma or end of string */
        while (*curr && *curr != ',')
            dest[i++] = *curr++;
        /* Remove trailing whitespace from the extracted word */
        while (i > 0 && isspace(dest[i - 1]))
            dest[--i] = '\0';
        /* Move past the comma if present */
        if (*curr == ',')
            curr++;
    }
    else
    {
        /* Extract word until we hit whitespace or end of string */
        while (*curr && !isspace(*curr))
            dest[i++] = *curr++;

        dest[i] = '\0'; /* Null-terminate the extracted word */
    }
    *src = curr; /* Update the source pointer to point after the extracted word */
    return i - 1;
}

/**
 * Function: count_and_validate_data_numbers
 * Description: Parses and validates a string of comma-separated numbers for .data directive.
 * @param line - The string containing the numbers to validate
 * @return The count of valid numbers, or -1 if syntax errors were found
 */
int count_and_validate_data_numbers(char *line)
{
    int count = 0;
    char *ptr = line;
    int expecting_comma = FALSE; /* Flag to track whether we expect a number or comma next */

    /* Skip leading whitespace */
    ptr = skip_whitespaces(ptr);

    /* Check if the line is empty */
    if (ptr == NULL || *ptr == '\0')
    {
        log_error(ERR_MISSING_OPERAND, 0, NULL, ".data");
        return 0;
    }

    /* Parse the line, alternating between expecting numbers and commas */
    while (*ptr != '\0')
    {
        if (!expecting_comma)
        {
            char *endptr;
            long val = strtol(ptr, &endptr, 10);

            /* Check for comma at start (syntax error) */
            if (*ptr == ',')
            {
                log_error(ERR_COMMA_AT_START, 0, NULL, ".data");
                return -1;
            }
            /* Check if parsing failed (no digits were converted) */
            if (ptr == endptr)
            {
                log_error(ERR_INVALID_NUMBER, 0, NULL, NULL);
                return -1;
            }
            /* Check for invalid characters after the number */
            if (*endptr != '\0' && !isspace(*endptr) && *endptr != ',')
            {
                log_error(ERR_INVALID_NUMBER, 0, NULL, "Float or invalid characters");
                return -1;
            }

            /* checks if the number value is exceeded 12 bit */
            if (val < -2048 || val > 2047)
            {
                log_error(ERR_DATA_OUT_OF_RANGE, 0, NULL, ptr);
                return -1;
            }

            /* Valid number found */
            count++;
            ptr = endptr;
            expecting_comma = TRUE; /* Next we expect a comma (or end of line) */
        }
        else
        {
            /* We're expecting a comma here */
            ptr = skip_whitespaces(ptr);
            /* If we've reached the end, that's fine (trailing number is valid) */
            if (*ptr == '\0')
                break;
            /* Check for the comma */
            if (*ptr == ',')
            {
                ptr++;
                expecting_comma = FALSE; /* Next we expect a number */
            }
            else
            {
                /* Found something other than a comma where comma was expected */
                log_error(ERR_MISSING_COMMA, 0, NULL, NULL);
                return -1;
            }
        }
    }
    /* Check if we ended while expecting a number (trailing comma error) */
    if (!expecting_comma)
    {
        log_error(ERR_COMMA_AT_END, 0, NULL, ".data");
        return -1;
    }
    return count;
}

/**
 * Function: is_register
 * Description: Checks if a string represents a valid register name (r0-r7).
 * @param op - The string to check
 * @return TRUE if the string is a valid register name, FALSE otherwise
 */
int is_register(const char *op)
{
    /* Check: length is 2, starts with 'r', and second char is digit 0-7 */
    return (strlen(op) == 2 && op[0] == 'r' && op[1] >= '0' && op[1] <= '7');
}

/**
 * Function: is_valid_addressing
 * Description: Checks if the addressing modes of the operands are legal for the given command.
 * The validation is based on the allowed addressing modes table from the project requirements.
 * @param opcode - The numeric code of the instruction (e.g., mov=0, lea=4).
 * @param op_count - How many operands the command expects (0, 1, or 2).
 * @param src - The addressing mode found for the source operand.
 * @param dest - The addressing mode found for the destination operand.
 * @return TRUE if the addressing modes are allowed for this opcode, otherwise FALSE
 */
boolean is_valid_addressing(int opcode, int op_count, addressing_mode src, addressing_mode dest)
{
    /* Check source operand (only relevant for commands that take 2 operands) */
    if (op_count == 2)
    {
        /* 'lea' command must use direct addressing for source */
        if (opcode == 4 && src != DIRECT_ADDR)
            return FALSE;

        /* 'mov', 'cmp', 'add', 'sub' cannot use index addressing for source */
        if ((opcode == 0 || opcode == 1 || opcode == 2) && src == INDEX_ADDR)
            return FALSE;
    }

    /* Check destination operand (relevant for commands that take 1 or 2 operands) */
    if (op_count >= 1)
    {
        if (opcode == 9)
        {
            /* 'jmp', 'bne', 'jsr' can only use direct (1) or index (2) addressing */
            if (dest != DIRECT_ADDR && dest != INDEX_ADDR)
                return FALSE;
        }
        else if (opcode == 1 || opcode == 13)
        {
            /* 'cmp', 'prn' can use anything EXCEPT index addressing (2) */
            if (dest == INDEX_ADDR)
                return FALSE;
        }
        else
        {
            /* All other commands (mov, add, sub, lea, clr, not, inc, dec, red) can only use direct (1) or register (3) addressing */
            if (dest != DIRECT_ADDR && dest != REGISTER_ADDR)
                return FALSE;
        }
    }
    /* All checks passed, the addressing modes are valid */
    return TRUE;
}

/**
 * Function: get_arg_mode
 * Description: Determines the addressing mode of an operand based on its syntax.
 * @param arg - The operand string to analyze
 * @return The addressing mode (IMMEDIATE_ADDR, REGISTER_ADDR, DIRECT_ADDR, or INDEX_ADDR)
 */
addressing_mode get_arg_mode(char *arg)
{
    /* Check for immediate addressing (#number) */
    if (*arg == '#')
        return IMMEDIATE_ADDR;

    /* Check for index/relative addressing (%label) */
    if (*arg == '%')
        return INDEX_ADDR;

    /* Check for register addressing (r0-r7) */
    if (is_register(arg))
        return REGISTER_ADDR;

    /* Default to direct addressing (label name) */
    return DIRECT_ADDR;
}

/**
 * Function: is_vaild_command_line
 * Description: Validates the operand count and comma placement for an instruction line.
 * @param cmd_name - The name of the command
 * @param args - The arguments string to validate
 * @param line - The line number for error reporting
 * @param filename - The filename for error reporting
 * @return TRUE if the command line is valid, FALSE otherwise
 */
boolean is_vaild_command_line(char *cmd_name, char *args, int line, char *filename)
{
    char *ptr = args;
    char curr_arg[MAX_LINE_LEN] = {0};
    int found_ops = 0, i;
    char arg1[MAX_LINE_LEN] = {0}; /* saving the first operand */
    char arg2[MAX_LINE_LEN] = {0}; /* saving the second operand */

    /* Look up the command information */
    const CmdInfo *cmd = find_cmd_info(cmd_name);

    /* Check if the command exists in the operations table */
    if (cmd == NULL)
        return log_error(ERR_UNKNOWN_INSTRUCTION, line, filename, cmd_name);

    /* Extract and count the operands */
    for (i = 0; i < cmd->op_count; i++)
    {
        curr_arg[0] = '\0';

        /* Parse the next argument, expecting comma for all but the last operand */
        get_next_word(&ptr, curr_arg, i < cmd->op_count - 1);

        if (i == 0)
            strcpy(arg1, curr_arg);
        if (i == 1)
            strcpy(arg2, curr_arg);
        /* Count non-empty arguments */
        found_ops += (curr_arg[0] != '\0') ? 1 : 0;
    }

    /* Check for extraneous text after all expected operands */
    ptr = skip_whitespaces(ptr);
    if (*ptr != '\0')
        return log_error(ERR_TOO_MANY_OPERANDS, line, filename, cmd_name);

    /* Check if we found fewer operands than required */
    if (found_ops < cmd->op_count)
        return log_error(ERR_MISSING_OPERAND, line, filename, cmd_name);

    /* Case 1: Command has exactly 2 operands */
    if (cmd->op_count == 2)
    {
        /* Get the addressing mode of each operand */
        addressing_mode src_mode = get_arg_mode(arg1);
        addressing_mode dest_mode = get_arg_mode(arg2);

        /* Check against the rules table */
        if (!is_valid_addressing(cmd->opcode, cmd->op_count, src_mode, dest_mode))
            return log_error(ERR_INVALID_OPERAND_TYPE, line, filename, cmd_name);
    }
    /* Case 2: Command has exactly 1 operand */
    else if (cmd->op_count == 1)
    {
        /* Get the addressing mode of the destination operand */
        addressing_mode dest_mode = get_arg_mode(arg1);
        /* Pass 0 for source because it doesn't matter for 1-operand commands */
        if (!is_valid_addressing(cmd->opcode, cmd->op_count, 0, dest_mode))
            return log_error(ERR_INVALID_OPERAND_TYPE, line, filename, cmd_name);
    }

    return TRUE;
}