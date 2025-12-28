#ifndef ERRORS_H
#define ERRORS_H

typedef enum
{
    ERR_NONE = 0,

    /* Syntax */
    ERR_LINE_TOO_LONG,
    ERR_SYNTAX_ERROR,
    ERR_EXTRA_TEXT_AFTER_CMD,
    ERR_MISSING_COMMA,
    ERR_MULTIPLE_COMMAS,
    ERR_COMMA_AT_START,
    ERR_COMMA_AT_END,

    /* Labels \ Symbols */
    ERR_LABEL_TOO_LONG,
    ERR_INVALID_LABEL_NAME,
    ERR_LABEL_ALREADY_DEFINED,
    ERR_LABEL_IS_KEYWORD,
    ERR_UNDEFINED_LABEL,
    ERR_SYMBOL_ALREADY_DEFINED,

    /* Operations \ Instruction */
    ERR_UNDEFINED_OPCODE,
    ERR_UNDEFINED_DIRECTIVE,
    ERR_MISSING_OPERAND,
    ERR_TOO_MANY_OPERANDS,
    ERR_INVALID_OPERAND_TYPE,
    ERR_UNKNOWN_INSTRUCTION,

    /* Data */
    ERR_INVALID_NUMBER,
    ERR_INVALID_STRING,
    ERR_DATA_OUT_OF_RANGE,

    /* System */
    ERR_MEMORY_ALLOCATION_FAILED,
    ERR_OPEN_FILE,

    /* Number of Errors */
    ERR_COUNT /* NEEDS TO BE IN THE END! */
} ErrorCode;

void log_error(ErrorCode code, int line_num, const char *filename, const char *info);

#endif