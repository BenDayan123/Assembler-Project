/*
 * File: main.c
 * Description: Entry point for the assembler program.
 * Orchestrates the three-phase assembly process: pre-assembly, first pass, and second pass.
 *
 * GitHub Repository: 'https://github.com/BenDayan123/Assembler-Project'
 */
#include <stdio.h>
#include "../headers/globals.h"
#include "../headers/utils.h"
#include "../headers/pre_assembler.h"
#include "../headers/first_pass.h"
#include "../headers/second_pass.h"
#include "../headers/error.h"
#include "../headers/symbol_table.h"

/**
 * Function: process_file
 * Description: Processes a single assembly source file through all assembly phases.
 * Executes pre-assembly (macro expansion), first pass (symbol table building),
 * and second pass (code generation) in sequence.
 * @param filename - The base name of the file to process (without extension)
 */
void process_file(char *filename)
{
    SymbolTable *table = NULL;
    int ICF, DCF; /* Final Instruction Counter and Data Counter */
    char *am_filename;

    /* Reset the global error flag for this file */
    was_error_found = FALSE;

    /* Create a new symbol table for this file */
    table = create_table();

    /* Check if table creation succeeded */
    if (!table)
    {
        log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, "Symbol Table");
        return;
    }

    /* Print header to indicate which file is being processed */
    printf("--- Processing file: '%s.as' ---\n", filename);

    /* Phase 1: Pre-assembler - expand macros and generate .am file */
    if (run_pre_assembler(filename))
    {
        /* Store the .am filename for potential cleanup */
        am_filename = create_file_path("output", filename, "am");

        /* Phase 2: First pass - build symbol table and calculate memory requirements */
        /* Only proceed to second pass if first pass succeeds and no errors happened */
        if (run_first_pass(filename, table, &ICF, &DCF) && !was_error_found)
            /* Phase 3: Second pass - encode instructions and generate output files */
            run_second_pass(filename, table, ICF, DCF);

        /* First pass failed - skip second pass */
        else
            log_custom_error("found in the first pass, skipping second pass...");
        free(am_filename);
    }

    /* Free the symbol table to prevent memory leaks */
    free_table(table);

    /* Print footer to indicate completion */
    printf("--- Finished: '%s.as' ---\n\n", filename);
}

/**
 * Function: main
 * Description: Entry point of the assembler program.
 * Processes each filename provided as a command-line argument.
 * @param argc - Number of command-line arguments (including program name)
 * @param argv - Array of command-line argument strings
 * @return 0 on successful completion
 */
int main(int argc, char *argv[])
{
    int i;
    /* Process each file provided as a command-line argument */
    /* Loop through argv (skipping argv[0] which is the program name) */
    for (i = 1; i < argc; i++)
    {
        /* Call the main processing function for each file */
        process_file(argv[i]);
    }
    /* Exit with success code */
    return 0;
}

/* ----- GitHub Repository: 'https://github.com/BenDayan123/Assembler-Project' ---- */