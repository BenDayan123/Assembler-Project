#include <stdio.h>
#include <string.h>
#include "../headers/second_pass.h"
#include "../headers/first_pass.h"
#include "../headers/node.h"
#include "../headers/globals.h"
#include "../headers/error.h"
#include "../headers/utils.h"

typedef struct MachineWord
{
    unsigned int code : 12;
    unsigned int ARE : 3;
} MachineWord;

static MachineWord code_image[MEMORY_SIZE];
static MachineWord data_image[MEMORY_SIZE];
static node *ext_list = NULL;

int encode_data_lines(char *args, const char *instruction, int *DC, int line_number)
{
    char *ptr = skip_whitespaces(args);
    if (ptr == NULL || *ptr == '\0')
        return FALSE;

    if (strcmp(instruction, ".string") == 0)
    {
        ptr = strchr(ptr, '"');
        if (ptr == NULL)
            return FALSE;
        ptr++;
        while (*ptr && *ptr != '"')
        {
            data_image[*DC].code = (unsigned int)(*ptr);
            data_image[*DC].ARE = 0;
            (*DC)++;
            ptr++;
        }
        data_image[*DC].code = 0;
        data_image[*DC].ARE = 0;
        (*DC)++;
    }

    if (strcmp(instruction, ".data") == 0)
    {
        while (*ptr)
        {
            char *end_ptr;
            long val;
            ptr = skip_whitespaces(ptr);
            if (*ptr == '\0')
                break;
            if (*ptr == ',')
            {
                ptr++;
                continue;
            }
            val = strtol(ptr, &end_ptr, 10);
            if (ptr == end_ptr)
                break;

            data_image[*DC].code = (unsigned int)val;
            data_image[*DC].ARE = 0;
            (*DC)++;
            ptr = end_ptr;
        }
    }

    return TRUE;
}

unsigned int build_word(int opcode, int funct, addressing_mode src_mode, addressing_mode dest_mode)
{
    /*
       11-8: Opcode
       7-4:  Funct
       3-2:  Src Mode
       1-0:  Dest Mode
    */

    unsigned int word = 0;
    word |= (opcode << 8);
    word |= (funct << 4);
    word |= (src_mode << 2);
    word |= (dest_mode << 0);
    return word;
}

void encode_operand(char *arg, SymbolTable *table, addressing_mode mode, int *IC, int line_number)
{
    switch (mode)
    {
    case IMMEDIATE_ADDR:
    {
        int val = atoi(arg + 1);
        code_image[*IC].code = val;
        code_image[*IC].ARE = ARE_ABSOLUTE;
        (*IC)++;
        break;
    }
    case REGISTER_ADDR:
    {
        int reg_num = atoi(arg + 1);
        code_image[*IC].code = (1 << reg_num);
        code_image[*IC].ARE = ARE_ABSOLUTE;
        (*IC)++;
        break;
    }
    case DIRECT_ADDR:
    {
        symbol *sym = find_symbol(table, arg);
        if (sym)
        {
            code_image[*IC].code = sym->address;
            if (sym->type == SYMBOL_EXTERN)
            {
                char IC_str[MAX_LINE_LEN];
                code_image[*IC].ARE = ARE_EXTERNAL;
                code_image[*IC].code = 0;
                sprintf(IC_str, "%d", *IC);
                if (ext_list == NULL)
                    ext_list = create_node(arg, IC_str);
                else
                    add_node(ext_list, create_node(arg, IC_str));
            }
            else
                code_image[*IC].ARE = ARE_RELOCATABLE;
        }
        else
            log_error(ERR_UNDEFINED_LABEL, line_number, NULL, arg);

        (*IC)++;
        break;
    }
    case INDEX_ADDR:
    {
        symbol *sym = find_symbol(table, arg + 1);
        if (sym)
        {
            if (sym->type == SYMBOL_EXTERN)
                log_error(ERR_INVALID_OPERAND_TYPE, line_number, NULL, "Relative addressing cannot be external");
            else
            {
                int diff = sym->address - *IC;
                code_image[*IC].code = diff;
                code_image[*IC].ARE = ARE_ABSOLUTE;
                (*IC)++;
            }
        }

        else
            log_error(ERR_INVALID_OPERAND_TYPE, line_number, NULL, arg);
        break;
    }
    default:
        break;
    }
}

boolean encode_instruction(char *line, SymbolTable *table, int *IC, int line_num)
{
    char op_name[MAX_LABEL_LEN], arg1[MAX_LABEL_LEN] = {0}, arg2[MAX_LABEL_LEN] = {0};
    char *ptr = line;
    unsigned int word;
    int src_mode = 0, dest_mode = 0;
    CmdInfo *cmd;

    get_next_word(&ptr, op_name, FALSE);
    cmd = find_cmd_info(op_name);
    if (cmd == NULL)
        return log_error(ERR_UNKNOWN_INSTRUCTION, line_num, NULL, op_name);

    if (cmd->op_count >= 1)
        get_next_word(&ptr, arg1, TRUE);
    if (cmd->op_count == 2)
        get_next_word(&ptr, arg2, TRUE);

    if (cmd->op_count == 2)
    {
        src_mode = get_arg_mode(arg1);
        dest_mode = get_arg_mode(arg2);
    }
    else if (cmd->op_count == 1)
    {
        src_mode = 0;
        dest_mode = get_arg_mode(arg1);
    }

    word = build_word(cmd->opcode, cmd->funct, src_mode, dest_mode);
    code_image[*IC].code = word;
    code_image[*IC].ARE = ARE_ABSOLUTE;
    (*IC)++;

    if (cmd->op_count == 2)
    {
        encode_operand(arg1, table, src_mode, IC, line_num);
        encode_operand(arg2, table, dest_mode, IC, line_num);
    }
    else if (cmd->op_count == 1)
        encode_operand(arg1, table, dest_mode, IC, line_num);
    return TRUE;
}

void write_object_file(char *filename, int ICF, int DCF)
{
    char *obj_filename = create_file_path("output", filename, "ob");
    FILE *obj_file = fopen(obj_filename, "w");
    int i;
    if (obj_file == NULL)
    {
        log_error(ERR_OPEN_FILE, 0, NULL, obj_filename);
        return;
    }
    fprintf(obj_file, "\t%d %d\n", ICF - IC_INIT_VALUE, DCF);

    for (i = IC_INIT_VALUE; i < ICF; i++)
    {
        int ARE = code_image[i].ARE;
        char ARE_char = ARE == ARE_ABSOLUTE ? 'A' : ARE == ARE_EXTERNAL ? 'E'
                                                                        : 'R';
        fprintf(obj_file, "%04d %03X %c\n", i, code_image[i].code & 0xFFF, ARE_char);
    }
    for (i = 0; i < DCF; i++)
        fprintf(obj_file, "%04d %03X A\n", ICF + i, data_image[i].code & 0xFFF);
    fclose(obj_file);
    free(obj_filename);
}

void write_entries_file(char *filename, SymbolTable *table)
{
    if (has_entries(table))
    {
        char *ent_filename = create_file_path("output", filename, "ent");
        FILE *ent_file = fopen(ent_filename, "w");
        int i;
        if (!ent_file)
        {
            log_error(ERR_OPEN_FILE, 0, NULL, ent_filename);
            return;
        }
        for (i = 0; i < table->count; i++)
        {
            if (table->symbols[i].is_entry)
                fprintf(ent_file, "%s %04d\n", table->symbols[i].name, table->symbols[i].address);
        }
        fclose(ent_file);
        free(ent_filename);
    }
}

void write_externals_file(char *filename, SymbolTable *table)
{
    if (ext_list != NULL)
    {
        char *ext_filename = create_file_path("output", filename, "ext");
        FILE *ext_file = fopen(ext_filename, "w");
        node *curr = ext_list;
        if (ext_file == NULL)
        {
            log_error(ERR_OPEN_FILE, 0, NULL, ext_filename);
            return;
        }
        while (curr != NULL)
        {
            int address = atoi(curr->content);
            fprintf(ext_file, "%s %04d\n", curr->key, address);
            curr = curr->next;
        }
        fclose(ext_file);
        free(ext_filename);
    }
}

/* ========================================= */
/* Main Logic                                */
/* ========================================= */

int run_second_pass(char *filename, SymbolTable *table, int ICF, int DCF)
{
    int IC = IC_INIT_VALUE, DC = 0;
    char line[MAX_LINE_LEN], curr_word[MAX_LINE_LEN];
    int line_number = 0;
    char *am_filename = create_file_path("output", filename, "am");
    FILE *am_file = fopen(am_filename, "r");

    memset(code_image, 0, sizeof(code_image));
    memset(data_image, 0, sizeof(data_image));
    ext_list = NULL;

    if (am_file == NULL)
        return log_error(ERR_OPEN_FILE, 0, NULL, am_filename);

    while (fgets(line, MAX_LINE_LEN, am_file))
    {
        char *ptr = skip_whitespaces(line);
        char first_word[MAX_LINE_LEN] = {0};
        line_number++;

        get_first_word(ptr, first_word);
        if (first_word[strlen(first_word) - 1] == ':')
            get_next_word(&ptr, first_word, FALSE);

        ptr = skip_whitespaces(ptr);
        if (*ptr == '\0')
            continue;

        if (*ptr == '.')
        {
            get_next_word(&ptr, curr_word, FALSE);
            if (strcmp(curr_word, ".entry") == 0)
            {
                char label[MAX_LABEL_LEN];
                get_next_word(&ptr, label, FALSE);
                if (find_symbol(table, label) == NULL)
                    return log_error(ERR_UNDEFINED_LABEL, line_number, am_filename, label);
                update_symbol(table, label, -1, TRUE);
            }
            if (strcmp(curr_word, ".data") == 0 || strcmp(curr_word, ".string") == 0)
                encode_data_lines(ptr, curr_word, &DC, line_number);
        }
        else
            encode_instruction(ptr, table, &IC, line_number);
    }
    write_object_file(filename, ICF, DCF);
    write_entries_file(filename, table);
    write_externals_file(filename, table);
    print_symbol_table(table);

    fclose(am_file);
    free_nodes(ext_list);
    ext_list = NULL;
    return TRUE;
}