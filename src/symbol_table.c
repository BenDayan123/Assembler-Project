/*
 * File: symbol_table.c
 * Description: Implements a dynamic symbol table using an dynamic array.
 * Supports adding, searching, and updating symbols (labels).
 */

#include <stdio.h>
#include <string.h>
#include "../headers/symbol_table.h"
#include "../headers/utils.h"
#include "../headers/globals.h"
#include "../headers/error.h"

#define INITIAL_CAPACITY 10

/**
 * Function: create_table
 * Description: Allocates memory for a new symbol table.
 * @return Pointer to the new table or NULL on failure.
 */
SymbolTable *create_table()
{
    /* Allocate memory for the table structure itself */
    /* This structure contains metadata (count, capacity) and a pointer to the array */
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));

    /* Check if the allocation succeeded */
    if (!table)
    {
        log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, NULL);
        return NULL;
    }

    /* Initialize the table's capacity to our starting size */
    /* This means we can store 10 symbols before needing to resize */
    table->capacity = INITIAL_CAPACITY;

    /* Start with zero symbols in the table */
    table->count = 0;

    /* Allocate memory for the actual array of symbols */
    /* We allocate space for INITIAL_CAPACITY symbol structures */
    table->symbols = (symbol *)malloc(table->capacity * sizeof(symbol));

    /* Check if the array allocation succeeded */
    if (!(table->symbols))
    {
        /* If it failed, we need to clean up the table structure too */
        log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, NULL);
        free(table);
        return NULL;
    }
    return table;
}

/**
 * Function: add_symbol
 * Description: Adds a new symbol to the table.
 * @param table - The symbol table to add to
 * @param name - The symbolic name (label) to store
 * @param address - The memory address this symbol refers to
 * @param type - The type of symbol (code, data, or external)
 * @param is_entry - Whether this symbol is marked as an entry point
 */
void add_symbol(SymbolTable *table, const char *name, int address, symbol_type type, boolean is_entry)
{
    /* Cache the current count for readability */
    int count = table->count;
    symbol *current;

    /* Check if the array is at full capacity */
    if (count == table->capacity)
    {
        /* We need to grow the array to accommodate the new symbol */
        /* Increase capacity by 1 (you could also double it for better performance) */
        int new_capacity = table->capacity + 1;

        /* Attempt to resize the array */
        /* realloc will either extend the existing block or allocate a new one */
        symbol *temp = (symbol *)realloc(table->symbols, new_capacity * sizeof(symbol));

        /* Check if reallocation succeeded */
        if (!temp)
        {
            /* Realloc failed - the original array is still valid, but we can't add more */
            log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, NULL);
            return;
        }

        /* Update the table to use the new, larger array */
        table->symbols = temp;
        table->capacity = new_capacity;
    }

    /* Get a pointer to the slot where we'll store this new symbol */
    current = &table->symbols[count];

    /* Allocate memory for the symbol name string */
    /* We need +1 byte for the null terminator '\0' */
    table->symbols[count].name = (char *)malloc(strlen(name) + 1);

    /* Check if string allocation succeeded */
    if (!(table->symbols[count].name))
    {
        log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, NULL);
        return;
    }
    /* Copy the name string into our newly allocated memory */
    strcpy(current->name, name);

    /* Store the symbol's properties */
    current->address = address;   /* Where in memory this symbol refers to */
    current->type = type;         /* Code, data, or external symbol */
    current->is_entry = is_entry; /* Is this an entry point for other files? */

    /* Increment the count since we've successfully added a symbol */
    table->count++;
}

/**
 * Function: find_symbol
 * Description: Searches for a symbol by name.
 * @param table - The symbol table to search in
 * @param name - The name of the symbol to find
 * @return Pointer to the symbol structure if found, or NULL if not found.
 */
symbol *find_symbol(SymbolTable *table, const char *name)
{
    int i;

    /* Iterate through all symbols in the table */
    for (i = 0; i < table->count; i++)
    {
        /* Compare the current symbol's name with the search target */
        /* strcmp returns 0 when strings are equal */
        if (strcmp(table->symbols[i].name, name) == 0)
            /* Found it! Return a pointer to this symbol */
            /* The caller can then read or modify the symbol's properties */
            return &table->symbols[i];
    }
    /* We've checked all symbols and didn't find a match */
    return NULL;
}

/**
 * Function: update_symbol
 * Description: Updates the type or entry flag of an existing symbol.
 *
 * @param table - The symbol table containing the symbol
 * @param name - The name of the symbol to update
 * @param type - New type value, or -1 to keep existing value
 * @param is_entry - New entry flag value, or -1 to keep existing value
 */
void update_symbol(SymbolTable *table, const char *name, int type, boolean is_entry)
{
    /* First, find the symbol in the table */
    symbol *sym = find_symbol(table, name);

    /* If the symbol doesn't exist, there's nothing to update */
    if (sym == NULL)
        return;

    /* Update the type only if a valid new type was provided */
    /* The check for >= 0 allows -1 to mean "don't change" */
    if (type >= 0)
        sym->type = type;

    /* Update the entry flag only if a valid new value was provided */
    if (is_entry >= 0)
        sym->is_entry = is_entry;
}

/**
 * Function: has_entries
 * Description: Checks if the table contains any entry symbols.
 *
 * @param table - The symbol table to check
 * @return TRUE if at least one entry symbol exists, FALSE otherwise
 */
boolean has_entries(SymbolTable *table)
{
    int i;
    /* Scan through all symbols looking for any with is_entry set to TRUE */
    for (i = 0; i < table->count; i++)
    {
        if (table->symbols[i].is_entry)
            return TRUE; /* Found at least one entry - we can return immediately */
    }
    /* No entry symbols found in the entire table */
    return FALSE;
}

/**
 * Function: free_table
 * Description: Frees the table and all string names inside it.
 * @param table - The symbol table to free
 */
void free_table(SymbolTable *table)
{
    int i;
    /* Validate that the table pointer is not NULL */
    if (!table)
        return;
    /* First, free all the dynamically allocated name strings */
    /* Each symbol has its own allocated string that needs to be freed */
    for (i = 0; i < table->count; i++)
        free(table->symbols[i].name);
    /* After freeing all the names, free the array of symbol structures */
    free(table->symbols);
    /* Finally, free the table structure itself */
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

        printf("| %-15s | %03d       | %-10s | %-10s \n",
               s->name,
               s->address,
               get_type_string(s->type),
               s->is_entry ? "Entry" : "");
    }

    printf("==============================================\n");
    printf("Total Symbols: %d\n\n", table->count);
}
