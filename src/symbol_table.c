#include <stdio.h>
#include <string.h>
#include "../headers/symbol_table.h"
#include "../headers/utils.h"
#include "../headers/globals.h"
#include "../headers/error.h"

#define INITIAL_CAPACITY 10

SymbolTable *create_table()
{
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (!table)
    {
        log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, NULL);
        return NULL;
    }
    table->capacity = INITIAL_CAPACITY;
    table->count = 0;

    table->symbols = (symbol *)malloc(table->capacity * sizeof(symbol));
    if (!(table->symbols))
    {
        log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, NULL);
        free(table);
    }
    return table;
}

void add_symbol(SymbolTable *table, const char *name, int address, symbol_type type, boolean is_entry)
{
    int count = table->count;
    symbol *current;
    if (count == table->capacity)
    {
        int new_capacity = table->capacity + 1;
        symbol *temp = (symbol *)realloc(table->symbols, new_capacity * sizeof(symbol));
        if (!temp)
        {
            log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, NULL);
            return;
        }
        table->symbols = temp;
        table->capacity = new_capacity;
    }
    current = &table->symbols[count];
    table->symbols[count].name = (char *)malloc(strlen(name) + 1);
    if (!(table->symbols[count].name))
    {
        log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, NULL);
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

void update_symbol(SymbolTable *table, const char *name, symbol_type type, boolean is_entry)
{
    symbol *sym = find_symbol(table, name);
    if (sym == NULL)
        return;
    if (type >= 0)
        sym->type = type;
    if (is_entry >= 0)
        sym->is_entry = is_entry;
}

boolean has_entries(SymbolTable *table)
{
    int i;
    for (i = 0; i < table->count; i++)
    {
        if (table->symbols[i].is_entry)
            return TRUE;
    }
    return FALSE;
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

/* TODO: Remove it before sending the final version */
/* For Debugging ONLY !! */
const char *get_type_string(symbol_type type)
{
    switch (type)
    {
    case SYMBOL_CODE:
        return "code";
    case SYMBOL_DATA:
        return "data";
    case SYMBOL_EXTERN:
        return "external";
    default:
        return "unknown";
    }
}

void print_symbol_table(SymbolTable *table)
{
    int i;
    if (table == NULL)
    {
        printf("Symbol table is NULL.\n");
        return;
    }

    printf("\n");
    printf("================ SYMBOL TABLE ================\n");

    printf("| %-15s | %-9s | %-10s |\n", "Symbol Name", "Address", "Type");
    printf("|-----------------|-----------|------------|\n");

    for (i = 0; i < table->count; i++)
    {
        symbol *s = &table->symbols[i];

        printf("| %-15s | %03d       | %-10s |\n",
               s->name,
               s->address,
               get_type_string(s->type));
    }

    printf("==============================================\n");
    printf("Total Symbols: %d\n\n", table->count);
}
