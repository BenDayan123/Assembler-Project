#include <stdio.h>
#include <string.h>
#include "../headers/second_pass.h"
#include "../headers/first_pass.h"
#include "../headers/globals.h"
#include "../headers/error.h"
#include "../headers/utils.h"

typedef struct MachineWord
{
    unsigned int code : 12;
    unsigned int ARE : 3;
} MachineWord;

static MachineWord code_image[MEMORY_SIZE];
static MachineWord data_image[MEMORY_SIZE];

int encode_data_lines(char *args, const char *instruction, int *DC, int line_number)
{
    char *ptr = skip_whitespaces(args);
    if (ptr == NULL || *ptr == '\0')
        return FALSE;

    if (strcmp(instruction, ".string") == 0)
    {
        ptr = strchr(ptr, '"');
        if (ptr == NULL)
            return FALSE;
        ptr++;
        while (*ptr && *ptr != '"')
        {
            data_image[*DC].code = (unsigned int)(*ptr);
            data_image[*DC].ARE = 0;
            (*DC)++;
            ptr++;
        }
        data_image[*DC].code = 0;
        data_image[*DC].ARE = 0;
        (*DC)++;
    }

    if (strcmp(instruction, ".data") == 0)
    {
        while (*ptr)
        {
            char *end_ptr;
            long val;
            ptr = skip_whitespaces(ptr);
            if (*ptr == '\0')
                break;
            if (*ptr == ',')
            {
                ptr++;
                continue;
            }
            val = strtol(ptr, &end_ptr, 10);
            if (ptr == end_ptr)
                break;

            data_image[*DC].code = (unsigned int)val;
            data_image[*DC].ARE = 0;
            (*DC)++;
            ptr = end_ptr;
        }
    }

    return TRUE;
}

unsigned int build_word(int opcode, int funct, addressing_mode src_mode, addressing_mode dest_mode)
{
    /*
       11-8: Opcode
       7-4:  Funct
       3-2:  Src Mode
       1-0:  Dest Mode
    */

    unsigned int word = 0;
    word |= (opcode << 8);
    word |= (funct << 4);
    word |= (src_mode << 2);
    word |= (dest_mode << 0);
    return word;
}

void write_entries_file(char *filename, SymbolTable *table)
{
    if (has_entries(table))
    {
        char *ent_filename = create_file_path("output", filename, "ent");
        FILE *ent_file = fopen(ent_filename, "w");
        int i;
        printf("true");
        if (!ent_file)
        {
            log_error(ERR_OPEN_FILE, 0, -1, ent_filename);
            return;
        }
        for (i = 0; i < table->count; i++)
        {
            if (table->symbols[i].is_entry)
                fprintf(ent_file, "%s %04d\n", table->symbols[i].name, table->symbols[i].address);
        }
        fclose(ent_file);
        free(ent_filename);
    }
}

/* ========================================= */
/* Main Logic                                */
/* ========================================= */

int run_second_pass(char *filename, SymbolTable *table)
{
    int IC = IC_INIT_VALUE, DC = 0;
    char line[MAX_LINE_LEN], curr_word[MAX_LINE_LEN];
    int line_number = 0;
    char *am_filename = create_file_path("output", filename, "am");
    FILE *am_file = fopen(am_filename, "r");

    if (am_file == NULL)
        return log_error(ERR_OPEN_FILE, 0, -1, am_filename);

    while (fgets(line, MAX_LINE_LEN, am_file))
    {
        char *ptr = skip_whitespaces(line);
        char first_word[MAX_LINE_LEN] = {0};
        line_number++;

        get_first_word(ptr, first_word);
        if (first_word[strlen(first_word) - 1] == ':')
            get_next_word(&ptr, first_word, FALSE);

        ptr = skip_whitespaces(ptr);
        if (*ptr == '\0')
            continue;

        if (*ptr == '.')
        {
            get_next_word(&ptr, curr_word, FALSE);
            if (strcmp(curr_word, ".entry") == 0)
            {
                char label[MAX_LABEL_LEN];
                get_next_word(&ptr, label, FALSE);
                if (find_symbol(table, label) == NULL)
                    return log_error(ERR_UNDEFINED_LABEL, line_number, am_filename, label);
                update_symbol(table, label, -1, TRUE);
            }
            if (strcmp(curr_word, ".data") == 0 || strcmp(curr_word, ".string") == 0)
                encode_data_lines(ptr, curr_word, &DC, line_number);
        }
    }
    write_entries_file(filename, table);
    fclose(am_file);
    return TRUE;
}