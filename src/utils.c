#include <stdio.h>
#include <stdlib.h>
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

char *handle_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL)
        print_error(1); /* NEEDS TO BE CHANGED! */
    return ptr;
}

char *filename_with_ext(char *filename, const char *ext)
{
    char *new_filename = (char *)handle_malloc(strlen(filename) + strlen(ext) + 2);
    strcpy(new_filename, filename);
    strcat(filename, ".");
    strncat(filename, ext, 6);
    return filename;
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