/*
 * File: first_pass.c
 * Description: Executes the first pass of the assembler.
 * It validates syntax, builds the symbol table (labels), and calculates
 * the Instruction Counter (IC) and Data Counter (DC).
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../headers/first_pass.h"
#include "../headers/globals.h"
#include "../headers/symbol_table.h"
#include "../headers/error.h"
#include "../headers/utils.h"

/**
 * Function: is_valid_label
 * Description: Checks if a string is a valid label name according to assembly rules.
 * @param label - The string to validate as a label name
 * @return TRUE if valid, FALSE otherwise
 */
boolean is_valid_label(char *label)
{
    char *temp = label;
    int i = 0;

    /* Check if label is NULL, empty, or doesn't start with a letter */
    if (temp == NULL || *temp == '\0' || !isalpha(temp[0]))
        return log_error(ERR_INVALID_LABEL_NAME, 0, NULL, label);

    /* Check if label exceeds maximum allowed length */
    if (strlen(label) > MAX_LABEL_LEN)
        return log_error(ERR_LABEL_TOO_LONG, 0, NULL, label);

    /* Check if label is a reserved keyword (register or command name) */
    if (is_register(label) || find_cmd_info(label) != NULL)
        return log_error(ERR_LABEL_IS_KEYWORD, 0, NULL, label);

    /* Verify that all characters are alphanumeric */
    while (temp[i] != '\0' && i < MAX_LINE_LEN)
    {
        if (!isalnum(temp[i++]))
            return log_error(ERR_INVALID_LABEL_NAME, 0, NULL, label);
    }
    /* TODO: Check if the label is not a register, command or instruction */
    return TRUE;
}

/**
 * Function: is_directive
 * Description: Checks if a word is a directive (starts with '.').
 * @param word - The word to check
 * @return TRUE if it's a directive, FALSE otherwise
 */
int is_directive(const char *word)
{
    return word != NULL && word[0] != '\0' && word[0] == '.';
}

/**
 * Function: calc_DC
 * Description: Calculates the number of memory words required for a data directive.
 * @param args - The arguments string (e.g., "6, 5, -2" for .data or "abc" for .string)
 * @param instruction - The type of directive ("data" or "string")
 * @return The number of words needed, or -1 on error
 */
int calc_DC(char *args, const char *instruction)
{
    /* Handle .string directive */
    if (strcmp(instruction, "string") == 0)
    {
        char *start, *end;
        /* Check for empty arguments */
        if (*args == '\0')
            return log_error(ERR_MISSING_OPERAND, 0, NULL, ".string");

        /* Find the opening and closing quotes */
        start = strchr(args, '"');
        end = strrchr(args, '"');

        /* Validate that both quotes exist and are different */
        if (end == NULL || start == NULL || end == start)
        {
            log_error(ERR_INVALID_STRING, 0, NULL, ".string");
            return -1;
        }
        /* Calculate the string length (closing quote - opening quote) */
        /* Add 1 for the null terminator that will be added */
        if (end > start)
            return end - start;
        log_error(ERR_INVALID_STRING, 0, NULL, ".string");
        return -1;
    }
    /* Handle .data directive */
    if (strcmp(instruction, "data") == 0)
        return count_and_validate_data_numbers(args);
    return -1;
}

/**
 * Function: calc_IC
 * Description: Calculates the number of memory words required for an instruction.
 * @param op_name - The name of the instruction (e.g., "mov", "add")
 * @param args - The arguments string
 * @return The total number of words (L) needed for this instruction
 */
int calc_IC(char *op_name, char *args)
{
    /* Look up the instruction in the operations table */
    const CmdInfo *inst = find_cmd_info(op_name);

    if (inst == NULL)
        return -1;

    /* Calculate total words: 1 base word + 1 word per operand */
    return 1 + inst->op_count;
}

/**
 * Function: update_data_symbols_address
 * Description: Updates addresses of data symbols at the end of the first pass.
 * Data symbols are placed after code in memory, so we add ICF to their addresses.
 * @param table - The symbol table containing all symbols
 * @param ICF - The final Instruction Counter value (where data section begins)
 */
void update_data_symbols_address(SymbolTable *table, int ICF)
{
    int i;
    /* Iterate through all symbols in the table */
    for (i = 0; i < table->count; i++)
    {
        symbol *sym = &table->symbols[i];
        /* Only update data symbols (code and external symbols stay as they are) */
        if (sym->type == SYMBOL_DATA)
            sym->address += ICF;
    }
}

/**
 * Function: handle_directive
 * Description: Processes assembler directives (.data, .string, .extern, .entry).
 * It parses the arguments, validates syntax, updates the Data Counter (DC),
 * and adds symbols to the symbol table when necessary.
 *
 * @param ptr        - Pointer to the string arguments (after the directive name).
 * @param type       - The directive type string (e.g., ".data", ".string").
 * @param table      - Pointer to the symbol table.
 * @param DC         - Pointer to the Data Counter (will be incremented for data).
 * @param is_label   - Flag indicating if a label was defined at the start of the line.
 * @param label_name - The name of the label (used only if is_label is TRUE).
 * @param line_num   - The current line number (for error reporting).
 * @param fname      - The source file name (for error reporting).
 *
 * @return TRUE if the line was identified and handled as a directive, FALSE otherwise.
 */
boolean handle_directive(char *ptr, char *type, SymbolTable *table, int *DC, boolean is_label, char *label_name, int line_num, char *fname)
{
    /* Skip the leading '.' from the directive name */
    char *directive_name = type + 1;
    /* Handle .data and .string directives */
    if (strcmp(directive_name, "data") == 0 || strcmp(directive_name, "string") == 0)
    {
        int size;
        /* If a label was found, add it to the symbol table as a data symbol */
        if (is_label)
        {
            /* Add data label to table with current DC */
            if (find_symbol(table, label_name) == NULL)
                add_symbol(table, label_name, *DC, SYMBOL_DATA, FALSE);
            else
                return log_error(ERR_SYMBOL_ALREADY_DEFINED, line_num, fname, label_name);
        }
        /* Calculate how many words this data takes and increment DC */
        size = calc_DC(ptr, directive_name);
        *DC += size > 0 ? size : 0;
        return TRUE;
    }
    /* Handle .extern directive */
    else if (strcmp(directive_name, "extern") == 0)
    {
        /* TODO: Merge the logic of .extern and .entry */
        char extern_name[MAX_LINE_LEN];
        /* Extract the external symbol name */
        get_next_word(&ptr, extern_name, FALSE);
        if (extern_name[0] == '\0')
            return log_error(ERR_MISSING_OPERAND, line_num, fname, ".extern requires a label");
        else
            add_symbol(table, extern_name, 0, SYMBOL_EXTERN, FALSE);
        return TRUE;
    }
    /* Handle .entry directive */
    else if (strcmp(directive_name, "entry") == 0)
    {
        /* Check syntax only (entry is handled in 2nd pass) */
        char curr_arg[MAX_LINE_LEN];
        get_next_word(&ptr, curr_arg, FALSE);
        if (curr_arg[0] == '\0')
            return log_error(ERR_MISSING_OPERAND, line_num, fname, ".entry requires a label");
        /* Check for extraneous text after the entry label */
        get_next_word(&ptr, curr_arg, FALSE);
        if (curr_arg[0] != '\0')
            return log_error(ERR_EXTRA_TEXT_AFTER_CMD, line_num, fname, curr_arg);
        return TRUE;
    }
    return FALSE;
}

/* ========================================= */
/* Main Logic                                */
/* ========================================= */

/**
 * Function: run_first_pass
 * Description: Reads the .am file and builds the symbol table.
 * @param filename - The name of the file (without extension).
 * @param table - Pointer to the symbol table.
 * @param ICF - Pointer to store the final IC.
 * @param DCF - Pointer to store the final DC.
 * @return TRUE on success, FALSE on failure.
 */
int run_first_pass(char *filename, SymbolTable *table, int *ICF, int *DCF)
{
    int IC = IC_INIT_VALUE, DC = 0;
    char line[MAX_LINE_LEN], curr_word[MAX_LINE_LEN];
    int line_number = 0, L;
    char *am_filename = create_file_path("output", filename, "am");
    FILE *file_in = fopen(am_filename, "r");

    if (file_in == NULL)
    {
        log_error(ERR_OPEN_FILE, 0, NULL, am_filename);
        return FALSE;
    }

    /* Process the file line by line */
    while (fgets(line, MAX_LINE_LEN, file_in))
    {
        boolean is_label_found = FALSE;
        char label_name[MAX_LABEL_LEN] = {0}, first_word[MAX_LINE_LEN] = {0};
        char *ptr = line;
        line_number++;
        first_word[0] = '\0';
        curr_word[0] = '\0';

        /* Check for lines that exceed the maximum allowed length */
        if (strlen(line) > MAX_LINE_LEN)
        {
            log_error(ERR_LINE_TOO_LONG, line_number, am_filename, NULL);
            continue;
        }

        /* Extract the first word from the line */
        get_first_word(ptr, first_word);

        /* Check for lines starting with a comma (syntax error) */
        if (first_word[0] == ',')
        {
            log_error(ERR_COMMA_AT_START, line_number, am_filename, NULL);
            continue;
        }

        /* --- Step 1: Label Identification --- */
        /* Check if the first word is a label (ends with ':') */
        if (first_word[strlen(first_word) - 1] == ':')
        {
            /* Extract the label name without the colon */
            get_next_word(&ptr, curr_word, FALSE);
            curr_word[strlen(curr_word) - 1] = '\0'; /* Remove the ':' */

            /* Validate the label name */
            if (is_valid_label(curr_word))
            {
                is_label_found = TRUE;
                strcpy(label_name, curr_word);
            }
            else
                continue; /* Skip this line if label is invalid */
        }

        /* Get the next word (either directive or instruction) */
        get_next_word(&ptr, curr_word, FALSE);

        /* --- Step 2: Directive Handling (.data, .string, .extern, .entry) --- */
        if (is_directive(curr_word))
            handle_directive(ptr, curr_word, table, &DC, is_label_found, label_name, line_number, am_filename);

        /* --- Step 3: Instruction Handling (Code) --- */
        else if (is_vaild_command_line(curr_word, ptr, line_number, am_filename))
        {
            /* If a label was found, add it to the symbol table as a code symbol */
            if (is_label_found)
            {
                /* Check if the symbol already exists */
                if (!find_symbol(table, label_name))
                    add_symbol(table, label_name, IC, SYMBOL_CODE, FALSE);
                else
                    log_error(ERR_SYMBOL_ALREADY_DEFINED, line_number, am_filename, label_name);
            }
            /* Calculate the instruction length and increment IC */
            L = calc_IC(curr_word, ptr);
            IC += L > 0 ? L : 0;
        }
    }
    /* After processing all lines, adjust data symbol addresses */
    /* Data symbols need to be offset by the final IC value */
    update_data_symbols_address(table, IC);

    /* Update the final counter values */
    *ICF = IC;
    *DCF = DC;

    fclose(file_in);
    return TRUE;
}