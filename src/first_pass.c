#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../headers/first_pass.h"
#include "../headers/globals.h"
#include "../headers/symbol_table.h"
#include "../headers/error.h"
#include "../headers/utils.h"

boolean is_label(char *label)
{
    char *temp = label;
    int i = 0;
    if (temp == NULL || *temp == '\0' || strlen(temp) > MAX_LABEL_LEN || !isalpha(temp[0]))
        return FALSE;
    while (temp[i] != '\0' && i < MAX_LINE_LEN)
    {
        if (!isalnum(temp[i++]))
            return FALSE;
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
        char string[MAX_LINE_LEN], *start, *end;
        get_next_word(&args, string, FALSE);
        start = strchr(string, '"');
        end = strrchr(string, '"');
        if (start && end && end > start)
            return end - start;
        log_error(ERR_INVALID_STRING, 0, NULL, NULL);
        return -1;
    }
    if (strcmp(type, "data") == 0)
        return count_and_validate_data_numbers(args);
    return -1;
}
int calc_IC(char *op_name, char *args)
{
    char *ptr = args;
    char arg1[MAX_LINE_LEN] = {0}, arg2[MAX_LINE_LEN] = {0};
    int found_ops = 0;
    CmdInfo *inst = find_cmd_info(op_name);

    if (inst == NULL)
    {
        printf("Error: Undefined instruction name '%s'.\n", op_name);
        return -1;
    }

    get_next_word(&ptr, arg1, inst->op_count > 0);
    get_next_word(&ptr, arg2, inst->op_count > 1);

    found_ops += (arg1[0] != '\0') ? 1 : 0;
    found_ops += (arg2[0] != '\0') ? 1 : 0;

    if (found_ops != inst->op_count)
    {
        printf("Error: '%s' expects %d operands, found %d.\n",
               op_name, inst->op_count, found_ops);
        return -1;
    }

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

int run_first_pass(char *am_filename)
{
    SymbolTable *table = create_table();
    int IC = IC_INIT_VALUE, DC = 0;
    char line[MAX_LINE_LEN], curr_word[MAX_LINE_LEN];
    int line_number = 0;
    FILE *file_in = fopen(am_filename, "r");

    if (file_in == NULL)
    {
        log_error(ERR_OPEN_FILE, 0, NULL, am_filename);
        fclose(file_in);
        return FALSE;
    }
    while (fgets(line, MAX_LINE_LEN, file_in))
    {
        boolean is_label_found = FALSE;
        char symbol_name[MAX_LABEL_LEN], first_word[MAX_LINE_LEN];
        char *ptr = line;

        line_number++;
        if (ptr == NULL || ptr[0] == '\0' || ptr[0] == ';')
            continue;

        get_first_word(ptr, first_word);

        /* Handle Label*/
        if (strchr(first_word, ':'))
        {
            get_next_word(&ptr, curr_word, FALSE);
            curr_word[strlen(curr_word) - 1] = '\0';
            if (is_label(curr_word))
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
                    {
                        char err_msg[MAX_LINE_LEN];
                        sprintf(err_msg, "name: '%s'", symbol_name);
                        log_error(ERR_SYMBOL_ALREADY_DEFINED, line_number, am_filename, err_msg);
                    }
                }
                size = calc_DC(ptr, curr_word);
                if (size > 0)
                    DC += size;
            }
            else if (strcmp(curr_word, "extern") == 0)
            {
                char extern_name[MAX_LINE_LEN];
                get_next_word(&ptr, extern_name, FALSE);
                add_symbol(table, extern_name, 0, SYMBOL_EXTERN, FALSE);
            }
            else if (strcmp(curr_word, "entry") == 0)
                continue;
        }

        /* Handle Commands */
        else
        {
            int size;
            if (is_label_found)
            {
                if (!find_symbol(table, symbol_name))
                    add_symbol(table, symbol_name, IC, SYMBOL_CODE, FALSE);
                else
                {
                    char err_msg[MAX_LINE_LEN];
                    sprintf(err_msg, "name: '%s'", symbol_name);
                    log_error(ERR_SYMBOL_ALREADY_DEFINED, line_number, am_filename, err_msg);
                }
            }

            size = calc_IC(curr_word, ptr);
            if (size > 0)
                IC += size;
        }
    }
    update_data_symbols_address(table, IC);
    print_symbol_table(table);
    return TRUE;
}