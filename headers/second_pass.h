#ifndef SECONDPASS_H
#define SECONDPASS_H
#include "../headers/symbol_table.h"

int run_second_pass(char *filename, SymbolTable *table, int ICF, int DCF);

#endif