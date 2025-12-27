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
            printf("realloc error...");
            return;
        }
        /* FIXME: handle realloc error !!!! */
        strcat(temp->content, line);
    }
}

boolean run_pre_assembler(char *as_file)
{
    FILE *file;
    char line[MAX_LINE_LEN], first_word[MAX_LINE_LEN];
    boolean inside_mcro = FALSE;
    node *macro_list = create_node(NULL, NULL);
    node *curr_macro = macro_list;

    file = fopen(as_file, "r");
    if (file == NULL)
    {
        printf("Could not open file '%s'\n\n", as_file);
        fclose(file);
        return FALSE;
    }
    while (fgets(line, MAX_LINE_LEN, file))
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
            add_node(curr_macro, new_macro);
            curr_macro = new_macro;
            inside_mcro = TRUE;
        }
        else if (inside_mcro)
            add_macro_line(curr_macro, skip);
    }
    macro_list = macro_list->next;
    print_list(macro_list);
    free_nodes(macro_list);
    fclose(file);
    return TRUE;
}