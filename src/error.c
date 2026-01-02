/*
 * File: error.c
 * Description: Centralized error handling for the assembler.
 * Defines all error messages and provides a function to log them.
 */

#include <stdio.h>
#include <stdlib.h>
#include "../headers/error.h"
#include "../headers/globals.h"

/* ANSI escape codes for colored terminal output */
#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

/* Global flag to track if any error has occurred during assembly */
boolean was_error_found = FALSE;

/* Array of error messages corresponding to ErrorCode enum values */
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

/**
 * Function: log_error
 * Description: Prints an error message to stdout with red color.
 * Sets the global error flag to TRUE.
 * @param code - The error code (enum).
 * @param line_num - The line number where error occurred.
 * @param filename - The file name.
 * @param info - Optional extra info string.
 * @return FALSE (always, for convenience in return statements).
 */
boolean log_error(ErrorCode code, int line_num, char *filename, const char *info)
{
    /* Set the global error flag to indicate that an error has occurred */
    was_error_found = TRUE;

    printf("%s", COLOR_RED); /* Begin printing in red color */
    /* Print file location if line number is provided */
    if (line_num > 0)
        printf("{%s:%d} ", filename != NULL ? filename : "", line_num);

    /* Look up and print the error message from the array */
    if (code >= 0 && code < ERR_COUNT)
        printf("Error: %s", error_messages[code]);
    else
        printf("Error: Unknown error code");

    /* Print additional context information if provided */
    if (info != NULL)
        printf(" (\"%s\")", info);
    /* End the error message and reset color to normal */
    printf(".%s\n", COLOR_RESET);
    return FALSE; /* Return FALSE to indicate failure */
}

/**
 * Function: log_custom_error
 * Description: Logs a custom, user-defined error message to the standard output.
 * @param msg - The custom error description string to print.
 * @return FALSE (always), to allow usage like: return log_custom_error("...");
 */
boolean log_custom_error(const char *msg)
{
    was_error_found = TRUE;
    printf("%sError: '%s'%s\n", COLOR_RED, msg, COLOR_RESET);
    return FALSE;
}