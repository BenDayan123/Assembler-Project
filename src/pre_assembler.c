#include <stdio.h>
#include <string.h>
#include "../headers/pre_assembler.h"
#include "../headers/utils.h"
#include "../headers/node.h"
#include "../headers/globals.h"

/* ========================================= */
/* Internal Helper Functions                 */
/* ========================================= */

void add_macro_line(node *macro, char *line)
{
    node *temp = macro;
    if (temp->content == NULL)
    {
        temp->content = (char *)handle_malloc(strlen(line) + 1);
        strcpy(macro->content, line);
    }
    else
    {
        int new_len = strlen(temp->content) + strlen(line) + 1;
        temp->content = (char *)realloc(temp->content, new_len);
        if (!(temp->content))
        {
            /* FIXME: handle realloc error !!!! */
            printf("Realloc Error...");
            return;
        }
        strcat(temp->content, line);
    }
}

/* ========================================= */
/* Main Logic                                */
/* ========================================= */

boolean run_pre_assembler(char *filename)
{
    FILE *file_in, *file_out;
    char *am_filename, *as_filename;
    char line[MAX_LINE_LEN], first_word[MAX_LINE_LEN];
    boolean inside_mcro = FALSE;
    node *macro_list = create_node(NULL, NULL);
    node *curr_macro = macro_list;

    as_filename = filename_with_ext(filename, "as");
    am_filename = filename_with_ext(filename, "am");

    file_in = fopen(as_filename, "r");
    if (file_in == NULL)
    {
        printf("Error: Could not open file '%s'\n\n", as_filename);
        fclose(file_in);
        return FALSE;
    }
    file_out = fopen(am_filename, "w");
    if (!file_out)
    {
        printf("Error: Cannot create file %s\n", am_filename);
        fclose(file_in);
        return FALSE;
    }
    while (fgets(line, MAX_LINE_LEN, file_in))
    {
        char *skip = skip_whitespaces(line);
        sscanf(skip, "%s", first_word);
        if (strcmp(first_word, "mcroend") == 0)
            inside_mcro = FALSE;
        else if (strcmp(first_word, "mcro") == 0)
        {
            char macro_name[MAX_LABEL_LEN];
            node *new_macro;
            sscanf(skip, "%*s %s", macro_name);
            new_macro = create_node(macro_name, NULL);
            add_node(macro_list, new_macro);
            curr_macro = new_macro;
            inside_mcro = TRUE;
        }
        else if (inside_mcro)
            add_macro_line(curr_macro, skip);
        else
        {
            node *found_macro = search_node(macro_list, first_word);
            fputs((found_macro == NULL) ? skip : found_macro->content, file_out);
        }
    }
    macro_list = macro_list->next;
    print_list(macro_list);
    free_nodes(macro_list);
    fclose(file_in);
    return TRUE;
}