#include <stdio.h>
#include "../headers/globals.h"
#include "../headers/utils.h"
#include "../headers/pre_assembler.h"
#include "../headers/first_pass.h"
#include "../headers/second_pass.h"
#include "../headers/error.h"
#include "../headers/symbol_table.h"

void process_file(char *filename)
{
    SymbolTable *table = NULL;
    int ICF, DCF;
    char *am_filename;

    was_error_found = FALSE;
    table = create_table();

    if (!table)
    {
        log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, "Symbol Table");
        return;
    }

    printf("--- Processing file: '%s.as' ---\n", filename);

    if (run_pre_assembler(filename))
    {
        am_filename = create_file_path("output", filename, "am");
        if (run_first_pass(filename, table, &ICF, &DCF) && !was_error_found)
            run_second_pass(filename, table, ICF, DCF);
        else
            log_custom_error("error found in the first pass, Skipping second pass...");
        free(am_filename);
    }

    free_table(table);
    printf("--- Finished: '%s.as' ---\n\n", filename);
}

int main(int argc, char *argv[])
{
    while (--argc > 0)
        process_file(argv[argc]);
    return 0;
}