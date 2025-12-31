#ifndef FIRSTPASS_H
#define FIRSTPASS_H
#include "../headers/symbol_table.h"

int is_directive(const char *word);
int run_first_pass(char *am_filename, SymbolTable *table);

#endif