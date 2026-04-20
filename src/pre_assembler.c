/*
 * File: pre_assembler.c
 * Description: Handles the macro expansion phase.
 * It reads the source file (.as), saves macro definitions, and replaces
 * macro calls with their content in a new file (.am).
 */

#include <stdio.h>
#include <string.h>
#include "../headers/pre_assembler.h"
#include "../headers/utils.h"
#include "../headers/node.h"
#include "../headers/globals.h"
#include "../headers/error.h"

/* ========================================= */
/* Internal Helper Functions                 */
/* ========================================= */

/**
 * Function: add_macro_line
 * Description: Appends a line of text to a macro's content.
 * Algorithm:
 * 1. Checks if the macro already has content.
 * 2. If not, allocates new memory.
 * 3. If yes, reallocates memory to fit the new line size + existing size.
 * @param macro - Pointer to the node representing the macro.
 * @param line - The string line to append.
 */
void add_macro_line(node *macro, char *line)
{
    node *temp = macro;
    /* Check if this is the first line being added to the macro */
    if (temp->content == NULL)
    {
        /* First line in the macro - allocate memory for it */
        /* +1 for the null terminator */
        temp->content = (char *)handle_malloc(strlen(line) + 1);
        strcpy(macro->content, line);
    }
    else
    {
        /* Macro already has content - need to append to existing content */
        /* Calculate the new length needed: existing content + new line + null terminator */
        int new_len = strlen(temp->content) + strlen(line) + 1;

        /* Expand the memory block to accommodate the new content */
        char *ptr = (char *)realloc(temp->content, new_len);
        if (!ptr)
        {
            log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, NULL);
            return;
        }
        temp->content = ptr;
        /* Append the new line to the existing content */
        strcat(temp->content, line);
    }
}

/* ========================================= */
/* Main Logic                                */
/* ========================================= */

/**
 * Function: run_pre_assembler
 * Description: The main logic for the pre-assembler phase.
 * Algorithm:
 * 1. Opens the input (.as) and output (.am) files.
 * 2. Reads the input file line by line.
 * 3. Checks if the line is a macro definition ("mcro") or end ("mcroend").
 * 4. If inside a macro definition, saves the line to memory.
 * 5. If it's a macro name, writes the saved macro content to the output file.
 * 6. Otherwise, writes the original line to the output file.
 * @param filename - The base name of the file to process.
 * @return TRUE if successful, FALSE otherwise.
 */
boolean run_pre_assembler(char *filename)
{
    FILE *file_in, *file_out;
    char *am_filename, *as_filename;
    char line[MAX_LINE_LEN], first_word[MAX_LINE_LEN];
    boolean inside_mcro = FALSE;                /* Flag to track if we are inside a macro block */
    node *macro_list = create_node(NULL, NULL); /* Head of the linked list for macros, start with dummy head */
    node *curr_macro = macro_list;

    /* Create file paths with extensions */
    as_filename = create_file_path(NULL, filename, "as");
    am_filename = create_file_path("output", filename, "am");

    /* Open the input file for reading */
    file_in = fopen(as_filename, "r");
    if (file_in == NULL)
        return log_error(ERR_OPEN_FILE, 0, NULL, as_filename);

    /* Open the output file for writing */
    file_out = fopen(am_filename, "w");
    if (!file_out)
        return log_error(ERR_OPEN_FILE, 0, NULL, am_filename);

    /* Process file line by line */
    while (fgets(line, MAX_LINE_LEN, file_in))
    {
        char *ptr = skip_whitespaces(line);
        first_word[0] = '\0';

        /* Skip empty lines or comment lines (starting with ';') */
        if (ptr == NULL || ptr[0] == '\0' || ptr[0] == ';')
            continue;

        /* Extract the first word from the line */
        sscanf(ptr, "%s", first_word);

        /* Case 1: End of macro definition */
        if (strcmp(first_word, "mcroend") == 0)
            inside_mcro = FALSE;

        /* Case 2: Start of macro definition */
        else if (strcmp(first_word, "mcro") == 0)
        {
            char macro_name[MAX_LABEL_LEN] = {0};
            node *new_macro;
            sscanf(ptr, "%*s %s", macro_name); /* Skip 'mcro' and get name */

            /* Ensure a macro name was actually provided */
            if (macro_name[0] == '\0')
            {
                log_error(ERR_SYNTAX_ERROR, 0, as_filename, "Macro name is missing");
                continue;
            }
            /* Create new node and add to list */
            new_macro = create_node(macro_name, NULL);

            /* checking if the macro is vaild - not a register or a command */
            if (is_register(macro_name) || find_cmd_info(macro_name) != NULL)
            {
                log_error(ERR_LABEL_IS_KEYWORD, 0, as_filename, macro_name);
                free_node(new_macro);
                continue; /* skip this macro save */
            }

            /* Only add the macro if it doesn't already exist in the list */
            /* This prevents duplicate macro definitions */
            if (search_node(macro_list, macro_name) == NULL)
                add_node(macro_list, new_macro);
            else
            {
                char msg[MAX_LINE_LEN] = {0};
                sprintf(msg, "Macro '%s' already defined", macro_name);
                log_custom_error(msg);
                free_node(new_macro);
            }
            /* Set curr_macro to point to this new macro for content collection */
            curr_macro = new_macro;
            inside_mcro = TRUE;
        }
        /* Case 3: Inside a macro definition - save the line */
        else if (inside_mcro)
            add_macro_line(curr_macro, ptr);
        /* Case 4: Normal line or macro usage */
        else
        {
            char macro_search_word[MAX_LINE_LEN] = {0};
            node *found_macro = NULL;
            boolean is_label = FALSE;

            /* Check if the first word is a label (ends with a colon) */
            if (strlen(first_word) > 0 && first_word[strlen(first_word) - 1] == ':')
            {
                is_label = TRUE;
                /* Extract the second word, which might be the macro name */
                /* %*s skips the first word (label), %s reads the second word */
                sscanf(ptr, "%*s %s", macro_search_word);
            }
            /* No label found, the first word might be the macro name */
            else
                strcpy(macro_search_word, first_word);

            /* Search for the macro in the list */
            found_macro = search_node(macro_list, macro_search_word);

            if (found_macro != NULL)
            {
                /* If a label is present before the macro, print the label first on a new line */
                if (is_label)
                    fprintf(file_out, "%s ", first_word);
                /* Print the expanded macro content only if it's not empty */
                if (found_macro->content != NULL)
                    fputs(found_macro->content, file_out);
                else
                    fputs("\n", file_out); /* Macro is empty, just print a newline */
            }
            /* Not a macro, write the original line as-is to the file */
            else
                fputs(ptr, file_out);
        }
    }
    /* Cleanup: free all allocated memory and close files */
    free_nodes(macro_list);
    fclose(file_in);
    fclose(file_out);
    free(as_filename);
    free(am_filename);

    return TRUE;
}