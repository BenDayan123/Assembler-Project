#include <stdio.h>
#include "../headers/pre_assembler.h"
#include "../headers/utils.h"
#include "../headers/globals.h"

boolean run_pre_assembler(char *as_file)
{
    FILE *file;
    char line[MAX_LINE_LEN];
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
        printf("->%s", skip);
    }
    fclose(file);
    return TRUE;
}