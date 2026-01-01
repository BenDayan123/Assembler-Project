#include <stdio.h>
#include "../headers/globals.h"
#include "../headers/utils.h"
#include "../headers/pre_assembler.h"
#include "../headers/first_pass.h"
#include "../headers/second_pass.h"
#include "../headers/error.h"
#include "../headers/symbol_table.h"

int main(int argc, char *argv[])
{
    SymbolTable *table = NULL;
    int ICF, DCF; /* Final Instruction Counter and Data Counter */
    while (--argc > 0)
    {
        char *filename = argv[argc];
        table = create_table();
        was_error_found = FALSE;

        if (table == NULL)
        {
            log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, "Symbol Table");
            continue;
        }

        run_pre_assembler(filename);
        if (run_first_pass(filename, table, &ICF, &DCF) && !was_error_found)
            run_second_pass(filename, table, ICF, DCF);
        else
            printf("Errors found in file '%s.am', Skipping second pass.\n", filename);

        /* Clean up memory before next iteration */
        free_table(table);
    }
    return 0;
}