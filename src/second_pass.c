#include <stdio.h>
#include <string.h>
#include "../headers/second_pass.h"
#include "../headers/globals.h"
#include "../headers/error.h"
#include "../headers/utils.h"

int run_second_pass(char *am_filename, SymbolTable *table)
{
    int IC = IC_INIT_VALUE, DC = 0;
    char line[MAX_LINE_LEN], curr_word[MAX_LINE_LEN];
    int line_number = 0, L;
    FILE *am_file = fopen(am_filename, "r");

    if (am_file == NULL)
    {
        log_error(ERR_OPEN_FILE, 0, NULL, am_filename);
        return FALSE;
    }
    while (fgets(line, MAX_LINE_LEN, am_file))
    {
        line_number++;
    }
}