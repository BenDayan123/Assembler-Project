#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../headers/first_pass.h"
#include "../headers/globals.h"
#include "../headers/symbol_table.h"
#include "../headers/error.h"
#include "../headers/utils.h"

boolean is_valid_label(char *label)
{
    char *temp = label;
    int i = 0;
    if (strlen(label) > MAX_LABEL_LEN)
        return log_error(ERR_LABEL_TOO_LONG, 0, NULL, label);

    if (temp == NULL || *temp == '\0' || !isalpha(temp[0]))
        return log_error(ERR_INVALID_LABEL_NAME, 0, NULL, label);
    while (temp[i] != '\0' && i < MAX_LINE_LEN)
    {
        if (!isalnum(temp[i++]))
            return log_error(ERR_INVALID_LABEL_NAME, 0, NULL, label);
    }
    /* TODO: Checked if it's not a register, command or instruction */
    return TRUE;
}

int is_directive(const char *word)
{
    return word != NULL && word[0] != '\0' && word[0] == '.';
}

int calc_DC(char *args, const char *type)
{
    if (strcmp(type, "string") == 0)
    {
        char *start, *end;
        if (*args == '\0')
            return log_error(ERR_MISSING_OPERAND, 0, NULL, ".string");
        start = strchr(args, '"');
        end = strrchr(args, '"');
        if (end == NULL || start == NULL || end == start)
        {
            log_error(ERR_INVALID_STRING, 0, NULL, ".string");
            return -1;
        }
        if (end > start)
            return end - start;
        log_error(ERR_INVALID_STRING, 0, NULL, ".string");
        return -1;
    }
    if (strcmp(type, "data") == 0)
        return count_and_validate_data_numbers(args);
    return -1;
}

int calc_IC(char *op_name, char *args)
{
    CmdInfo *inst = find_cmd_info(op_name);
    if (inst == NULL)
        return -1;
    /*

    char *ptr = args;
    char arg1[MAX_LINE_LEN] = {0}, arg2[MAX_LINE_LEN] = {0};
    int found_ops = 0;



    get_next_word(&ptr, arg1, inst->op_count > 0);
    get_next_word(&ptr, arg2, inst->op_count > 1);

    found_ops += (arg1[0] != '\0') ? 1 : 0;
    found_ops += (arg2[0] != '\0') ? 1 : 0;
    */

    return 1 + inst->op_count;
}

void update_data_symbols_address(SymbolTable *table, int final_IC)
{
    int i;
    for (i = 0; i < table->count; i++)
    {
        symbol *sym = &table->symbols[i];
        if (sym->type == SYMBOL_DATA)
            sym->address += final_IC;
    }
}

/* ========================================= */
/* Main Logic                                */
/* ========================================= */

int run_first_pass(char *am_filename, SymbolTable *table)
{
    int IC = IC_INIT_VALUE, DC = 0;
    char line[MAX_LINE_LEN], curr_word[MAX_LINE_LEN];
    int line_number = 0, L;
    FILE *file_in = fopen(am_filename, "r");

    if (file_in == NULL)
    {
        log_error(ERR_OPEN_FILE, 0, NULL, am_filename);
        return FALSE;
    }
    while (fgets(line, MAX_LINE_LEN, file_in))
    {
        boolean is_label_found = FALSE;
        char symbol_name[MAX_LABEL_LEN] = {0}, first_word[MAX_LINE_LEN] = {0};
        char *ptr = line;
        line_number++;
        first_word[0] = '\0';
        curr_word[0] = '\0';

        if (strlen(line) > MAX_LINE_LEN)
        {
            log_error(ERR_LINE_TOO_LONG, line_number, am_filename, NULL);
            continue;
        }

        get_first_word(ptr, first_word);

        if (first_word[0] == ',')
        {
            log_error(ERR_COMMA_AT_START, line_number, am_filename, NULL);
            continue;
        }

        /* Handle Label*/
        if (strchr(first_word, ':'))
        {
            get_next_word(&ptr, curr_word, FALSE);
            curr_word[strlen(curr_word) - 1] = '\0';
            if (is_valid_label(curr_word))
            {
                is_label_found = TRUE;
                strcpy(symbol_name, curr_word);
            }
            else
                continue;
        }

        get_next_word(&ptr, curr_word, FALSE);
        /* Handle Instruction */
        if (is_directive(curr_word))
        {
            /* Skip the first dot */
            memmove(curr_word, curr_word + 1, strlen(curr_word));
            if (strcmp(curr_word, "data") == 0 || strcmp(curr_word, "string") == 0)
            {
                int size;
                if (is_label_found)
                {
                    if (find_symbol(table, symbol_name) == NULL)
                        add_symbol(table, symbol_name, DC, SYMBOL_DATA, FALSE);
                    else
                        log_error(ERR_SYMBOL_ALREADY_DEFINED, line_number, am_filename, symbol_name);
                }
                size = calc_DC(ptr, curr_word);
                DC += size > 0 ? size : 0;
            }
            else if (strcmp(curr_word, "extern") == 0)
            {
                /* TODO: Merge the logic of .extern and .entry */
                char extern_name[MAX_LINE_LEN];
                get_next_word(&ptr, extern_name, FALSE);
                if (extern_name[0] == '\0')
                    log_error(ERR_MISSING_OPERAND, line_number, am_filename, ".extern requires a label");
                else
                    add_symbol(table, extern_name, 0, SYMBOL_EXTERN, FALSE);
            }
            else if (strcmp(curr_word, "entry") == 0)
            {
                char curr_arg[MAX_LINE_LEN];
                get_next_word(&ptr, curr_arg, FALSE);
                if (curr_arg[0] == '\0')
                    log_error(ERR_MISSING_OPERAND, line_number, am_filename, ".entry requires a label");
                get_next_word(&ptr, curr_arg, FALSE);
                if (curr_arg[0] != '\0')
                    log_error(ERR_EXTRA_TEXT_AFTER_CMD, line_number, am_filename, curr_arg);
            }
        }

        /* Handle Commands */
        else if (is_vaild_command_line(curr_word, ptr, line_number, am_filename))
        {
            if (is_label_found)
            {
                if (!find_symbol(table, symbol_name))
                    add_symbol(table, symbol_name, IC, SYMBOL_CODE, FALSE);
                else
                    log_error(ERR_SYMBOL_ALREADY_DEFINED, line_number, am_filename, symbol_name);
            }
            L = calc_IC(curr_word, ptr);
            IC += L > 0 ? L : 0;
        }
    }
    update_data_symbols_address(table, IC);
    print_symbol_table(table);
    fclose(file_in);
    return TRUE;
}