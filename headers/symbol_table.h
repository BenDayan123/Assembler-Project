#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "./globals.h"

typedef enum
{
    SYMBOL_CODE,
    SYMBOL_DATA,
    SYMBOL_EXTERN
} symbol_type;

typedef struct symbol
{
    char *name;
    int address;
    symbol_type type;
    boolean is_entry;
} symbol;

typedef struct
{
    symbol *symbols;
    int count;
    int capacity;
} SymbolTable;

SymbolTable *create_table();
void add_symbol(SymbolTable *table, const char *name, int address, symbol_type type, boolean is_entry);
symbol *find_symbol(SymbolTable *table, const char *name);
void free_table(SymbolTable *table);
void update_symbol(SymbolTable *table, const char *name, int type, boolean is_entry);
boolean has_entries(SymbolTable *table);

#endif