#include <stdio.h>
#include <stdlib.h>
#include "../headers/error.h"
#include "../headers/globals.h"

#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

static const char *error_messages[] = {
    "No error",

    /* Syntax */
    "Line is too long (max 80 chars)",
    "General syntax error",
    "Extraneous text after end of command",
    "Missing comma between operands",
    "Multiple consecutive commas",
    "Line starts with a comma",
    "Line ends with a comma",

    /* Labels \ Symbols */
    "Label name exceeds maximum length of 31 characters",
    "Label name contains invalid characters or format",
    "Label has already been defined",
    "Label name cannot be a reserved keyword",
    "Use of undefined label",
    "Symbol is already defined in the symbol table",

    /* Operations \ Instructions */
    "Undefined instruction name",
    "Undefined directive",
    "Missing operand for instruction",
    "Too many operands for instruction",
    "Operand addressing mode is invalid for this instruction",
    "Unknown instruction name",

    /* Data */
    "Expected an integer number",
    "String must be enclosed in double quotes",
    "Number is out of range for 12-bit representation",

    /* System */
    "Fatal: Memory allocation failed",
    "Could not open file"};

boolean log_error(ErrorCode code, int line_num, char *filename, const char *info)
{
    printf("%s", COLOR_RED);
    if (line_num > 0)
        printf("{%s:%d} ", filename != NULL ? filename : "", line_num);
    if (code >= 0 && code < ERR_COUNT)
        printf("Error: %s", error_messages[code]);
    else
        printf("Error: Unknown error code");
    if (info != NULL)
        printf(" (\"%s\")", info);
    printf(".%s\n", COLOR_RESET);
    return FALSE;
}