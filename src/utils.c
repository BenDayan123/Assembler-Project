#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../headers/error.h"
#include "../headers/globals.h"

/*
 * The actual dictionary implementation.
 * -> NOTE: THE ORDER MUST MATCH THE 'opcode_name' ENUM IN 'globals.h' FILE! <-
 */
const CmdInfo operations[] = {
    /* { "name", opcode, funct, operand_count } */
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
    {"stop", 15, 0, 0}};

char *convert_to_binary(unsigned int num, unsigned int binary_length)
{
    unsigned int num_bits = 0; /* Count the number of bits */
    char *binary = NULL;       /* Initialize binary pointer to NULL */
    unsigned int temp = num;   /* Initialize temp to num */
    int i = 0;

    if (num == 0)
        return "0";

    if (binary_length > 0)
        num_bits = binary_length;
    else
    {
        /* Calculate the number of bits needed */
        while (temp > 0)
        {
            temp >>= 1; /* Right shift to divide by 2 */
            num_bits++;
        }
    }

    binary = (char *)malloc((num_bits + 1) * sizeof(char)); /* +1 for null terminator */
    if (binary == NULL)
        return NULL;         /* Memory allocation failed */
    binary[num_bits] = '\0'; /* Null terminate the string */

    for (i = num_bits - 1; i >= 0; i--)
    {
        binary[i] = (num & 1) ? '1' : '0'; /* Check the least significant bit */
        num >>= 1;                         /* Right shift to process the next bit */
    }

    return binary;
}

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
        print_error(1); /* NEEDS TO BE CHANGED! */
    return ptr;
}

char *filename_with_ext(char *filename, const char *ext)
{
    char *new_filename = (char *)handle_malloc(strlen(filename) + strlen(ext) + 1);
    sprintf(new_filename, "%s.%s", filename, ext);
    return new_filename;
}

char *join_path_and_ext(char *filename, const char *ext, const char *path)
{
    char *new_filename = (char *)handle_malloc(strlen(filename) + strlen(ext) + strlen(path) + 1);
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
        return;
    memset(dest, '\0', strlen(dest));
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

    if (*ptr == '\0')
        return 0;

    while (*ptr != '\0')
    {
        if (!expecting_comma)
        {
            char *end_ptr;
            strtol(ptr, &end_ptr, 10);
            if (*ptr == ',')
            {
                printf("Error: Unexpected comma (consecutive or leading).\n");
                return -1;
            }

            if (ptr == end_ptr)
            {
                printf("Error: Expected a number, found '%c'.\n", *ptr);
                return -1;
            }

            count++;
            ptr = end_ptr;
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
                printf("Error: Missing comma between numbers.\n");
                return -1;
            }
        }
    }

    if (!expecting_comma)
    {
        printf("Error: Trailing comma without a number.\n");
        return -1;
    }

    return count;
}

int is_register(const char *op)
{
    return (strlen(op) == 2 && op[0] == 'r' && op[1] >= '0' && op[1] <= '7');
}