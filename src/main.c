#include <stdio.h>
#include "../headers/globals.h"
#include "../headers/utils.h"
#include "../headers/pre_assembler.h"
#include "../headers/first_pass.h"
#include "../headers/second_pass.h"
#include "../headers/symbol_table.h"

int main(int argc, char *argv[])
{
    SymbolTable *table = NULL;
    int ICF, DCF;
    while (--argc > 0)
    {
        char *filename = argv[argc];
        table = create_table();

        run_pre_assembler(filename);
        run_first_pass(filename, table, &ICF, &DCF);
        run_second_pass(filename, table, ICF, DCF);

        free_table(table);
    }
    return 0;
}