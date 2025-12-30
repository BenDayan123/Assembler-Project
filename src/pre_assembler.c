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
            log_error(ERR_MEMORY_ALLOCATION_FAILED, 0, NULL, NULL);
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
    am_filename = join_path_and_ext(filename, "am", "output");

    file_in = fopen(as_filename, "r");
    if (file_in == NULL)
        return log_error(ERR_OPEN_FILE, 0, NULL, as_filename);
    file_out = fopen(am_filename, "w");
    if (!file_out)
        return log_error(ERR_OPEN_FILE, 0, NULL, am_filename);
    while (fgets(line, MAX_LINE_LEN, file_in))
    {
        char *ptr = skip_whitespaces(line);
        first_word[0] = '\0';
        if (ptr == NULL || ptr[0] == '\0' || ptr[0] == ';')
            continue;
        sscanf(ptr, "%s", first_word);
        if (strcmp(first_word, "mcroend") == 0)
            inside_mcro = FALSE;
        else if (strcmp(first_word, "mcro") == 0)
        {
            char macro_name[MAX_LABEL_LEN] = {0};
            node *new_macro;
            sscanf(ptr, "%*s %s", macro_name);
            new_macro = create_node(macro_name, NULL);
            add_node(macro_list, new_macro);
            curr_macro = new_macro;
            inside_mcro = TRUE;
        }
        else if (inside_mcro)
            add_macro_line(curr_macro, ptr);
        else
        {
            node *found_macro = search_node(macro_list, first_word);
            fputs((found_macro == NULL) ? ptr : found_macro->content, file_out);
        }
    }
    macro_list = macro_list->next;
    free_nodes(macro_list);
    fclose(file_in);
    fclose(file_out);
    return TRUE;
}