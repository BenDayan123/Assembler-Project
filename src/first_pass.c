#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../headers/first_pass.h"
#include "../headers/globals.h"
#include "../headers/symbol_table.h"
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
    // TODO: Checked if it's not a register, command or instruction
    return TRUE;
}

int is_directive(const char *word)
{
    return word != NULL && word[0] != '\0' && word[0] == '.';
}

int calc_DC(char *args, const char *type)
{
    int count = 0;

    if (strcmp(type, "string") == 0)
    {
        char *start = strchr(args, '"'); /* Search from the left */
        char *end = strrchr(args, '"');  /* Search from the right */
        if (start && end && start < end)
            return end - start;
        return -1;
    }
    if (strcmp(type, "data") == 0)
    {
        /* Needs to be completed */
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
        printf("Error: Could not open file '%s'\n\n", am_filename);
        fclose(file_in);
        return FALSE;
    }
    while (fgets(line, MAX_LINE_LEN, file_in))
    {
        boolean is_label_found = FALSE;
        char symbol_name[MAX_LABEL_LEN];
        char *curr_pos = line;

        line_number++;
        get_first_word(curr_pos, curr_word);

        if (curr_pos == NULL || curr_pos[0] == '\0' || curr_pos[0] == ';')
            continue;

        /* Handle Label*/
        if (strchr(curr_word, ':'))
        {
            curr_word[strlen(curr_word) - 1] = '\0';
            if (is_label(curr_word))
            {
                printf("Valid => '%s'\n", curr_word);
                is_label_found = TRUE;
                strcpy(symbol_name, curr_word);
                curr_pos = strchr(curr_pos, ':') + 1;
                curr_pos = skip_whitespaces(curr_pos);
            }
            else
                printf("Invalid => '%s'\n", curr_word);
        }

        get_next_word(curr_pos, curr_word);

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
                    if (!find_symbol(table, symbol_name))
                        add_symbol(table, symbol_name, DC, SYMBOL_DATA, FALSE);
                    else
                    {
                        // FIXME: Handle this error
                        printf("Error: in .data or .string\n");
                    }
                }
                size = calc_DC(curr_pos, curr_word);
                if (size < 0)
                    printf("Error: Invalid data format.\n");
                else
                    DC += size;
            }
            else if (strcmp(curr_word, "extern") == 0)
            {
                char ext_val[MAX_LABEL_LEN];
                curr_pos = skip_whitespaces(curr_pos);
                get_next_word(curr_pos, ext_val);

                add_symbol(table, ext_val, 0, SYMBOL_EXTERN, FALSE);
            }
            else if (strcmp(curr_word, "entry") == 0)
                continue;
        }
    }
    return TRUE;
}