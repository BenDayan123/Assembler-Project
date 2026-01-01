#ifndef FIRSTPASS_H
#define FIRSTPASS_H
#include "../headers/symbol_table.h"

int is_directive(const char *word);
int run_first_pass(char *filename, SymbolTable *table, int *ICF, int *DCF);

#endif