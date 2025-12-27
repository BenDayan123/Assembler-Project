#include <stdio.h>
#include <string.h>
#include "../headers/symbol_table.h"
#include "../headers/utils.h"
#include "../headers/globals.h"

#define INITIAL_CAPACITY 10

SymbolTable *create_table()
{
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (!table)
    {
        printf("Error: Memory allocation error\n");
        return NULL;
    }
    table->capacity = INITIAL_CAPACITY;
    table->count = 0;

    table->symbols = (SymbolTable *)malloc(table->capacity * sizeof(symbol));
    if (!(table->symbols))
    {
        printf("Error: Memory allocation error\n");
        free(table);
    }
    return table;
}

void add_symbol(SymbolTable *table, const char *name, int address, symbol_type type, boolean is_entry)
{
    int count = table->count;
    symbol *current = &table->symbols[count];
    if (count == table->capacity)
    {
        int new_capacity = table->capacity + 1;
        symbol *temp = (symbol *)realloc(table->symbols, new_capacity * sizeof(symbol));
        if (!temp)
        {
            printf("Error: Memory allocation error\n");
            return;
        }
        table->symbols = temp;
        table->capacity = new_capacity;
    }
    table->symbols[count].name = (char *)malloc(strlen(name) + 1);
    if (!(table->symbols[count].name))
    {
        printf("Error: Memory allocation error\n");
        return;
    }
    strcpy(current->name, name);
    current->address = address;
    current->type = type;
    current->is_entry = is_entry;
    table->count++;
}

symbol *find_symbol(SymbolTable *table, const char *name)
{
    int i;
    for (i = 0; i < table->count; i++)
    {
        if (strcmp(table->symbols[i].name, name) == 0)
            return &table->symbols[i];
    }
    return NULL;
}

void free_table(SymbolTable *table)
{
    int i;
    if (!table)
        return;
    for (i = 0; i < table->count; i++)
        free(table->symbols[i].name);
    free(table->symbols);
    free(table);
}