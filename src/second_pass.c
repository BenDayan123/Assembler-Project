/*
 * File: second_pass.c
 * Description: Executes the second pass of the assembler.
 * It encodes the instructions into machine language (binary/hex),
 * resolves label addresses, and generates the final output files.
 */

#include <stdio.h>
#include <string.h>
#include "../headers/second_pass.h"
#include "../headers/first_pass.h"
#include "../headers/node.h"
#include "../headers/globals.h"
#include "../headers/error.h"
#include "../headers/utils.h"

/* Structure for a 12-bit word with A.R.E attributes */
typedef struct MachineWord
{
    unsigned int code : 12; /* The actual 12-bit instruction or data value */
    unsigned int ARE : 3;   /* Addressing type: Absolute, Relocatable, or External */
} MachineWord;

/* Arrays to hold the encoded machine code and data */
static MachineWord code_image[MEMORY_SIZE];
static MachineWord data_image[MEMORY_SIZE];

/* Linked list for tracking external symbol references */
static node *ext_list = NULL;

/**
 * Function: encode_data_lines
 * Description: Encodes .data and .string directives into binary format in the data image.
 * @param args - The arguments string containing the data values
 * @param instruction - The directive type (".data" or ".string")
 * @param DC - Pointer to the Data Counter (updated as data is encoded)
 * @param line_number - Current line number for error reporting
 * @return TRUE if successful, FALSE otherwise
 */
boolean encode_data_lines(char *args, const char *directive, int *DC, int line_number)
{
    char *ptr = skip_whitespaces(args);
    /* Validate that we have arguments to process */
    if (ptr == NULL || *ptr == '\0')
        return FALSE;

    /* Handle .string directive: Convert each character to its ASCII value */
    if (strcmp(directive, ".string") == 0)
    {
        ptr = strchr(ptr, '"'); /* Find the opening quote */
        if (ptr == NULL)
            return FALSE;
        ptr++; /* Move past the opening quote */

        /* Process each character in the string */
        while (*ptr && *ptr != '"')
        {
            /* Store the ASCII value of the character */
            data_image[*DC].code = (unsigned int)(*ptr);
            data_image[*DC].ARE = 0; /* Data has no A.R.E attributes */
            (*DC)++;
            ptr++;
        }
        /* Add null terminator to end the string */
        data_image[*DC].code = 0;
        data_image[*DC].ARE = 0;
        (*DC)++;
    }

    /* Handle .data directive: Convert comma-separated numbers */
    if (strcmp(directive, ".data") == 0)
    {
        while (*ptr)
        {
            char *end_ptr;
            long val;
            ptr = skip_whitespaces(ptr);
            if (*ptr == '\0')
                break;
            /* Skip commas */
            if (*ptr == ',')
            {
                ptr++;
                continue;
            }
            /* Parse the integer value */
            val = strtol(ptr, &end_ptr, 10);
            if (ptr == end_ptr) /* Check if we can't parsed a number */
                break;

            /* Store the number in the data image */
            data_image[*DC].code = (unsigned int)val;
            data_image[*DC].ARE = 0;
            (*DC)++;
            /* Move to the position after the parsed number */
            ptr = end_ptr;
        }
    }

    return TRUE;
}

/**
 * Function: build_word
 * Description: Constructs the first word of an instruction using bitwise operations.
 * @param opcode - The operation code (bits 8-11)
 * @param funct - The function code for operations that share an opcode (bits 4-7)
 * @param src_mode - The source operand addressing mode (bits 2-3)
 * @param dest_mode - The destination operand addressing mode (bits 0-1)
 * @return The complete 12-bit instruction word
 */
unsigned int build_word(int opcode, int funct, addressing_mode src_mode, addressing_mode dest_mode)
{
    unsigned int word = 0;
    /* Build the instruction word by shifting each field into its correct position */
    word |= (opcode << 8);    /* Bits 8-11: opcode */
    word |= (funct << 4);     /* Bits 4-7: function code */
    word |= (src_mode << 2);  /* Bits 2-3: source addressing mode */
    word |= (dest_mode << 0); /* Bits 0-1: destination addressing mode */
    return word;
}

/**
 * Function: encode_operand
 * Description: Generates the extra machine word for an operand based on its addressing mode.
 * @param arg - The operand string.
 * @param table - Symbol table.
 * @param mode - Addressing mode (0, 1, 2, 3).
 * @param IC - Pointer to Instruction Counter.
 */
void encode_operand(char *arg, SymbolTable *table, addressing_mode mode, int *IC, int line_number)
{
    switch (mode)
    {
    case IMMEDIATE_ADDR: /* #number - immediate value */
    {
        /* Parse the number (skip the '#' prefix) */
        int val = atoi(arg + 1);

        /* Store the immediate value in the code image */
        code_image[*IC].code = val;
        code_image[*IC].ARE = ARE_ABSOLUTE; /* Immediate values are absolute */
        (*IC)++;
        break;
    }

    case REGISTER_ADDR: /* rX - register operand */
    {
        /* Extract the register number (skip the 'r' prefix) */
        int reg_num = atoi(arg + 1);

        /* Create a bitmask with bit N set for register rN */
        code_image[*IC].code = (1 << reg_num);
        code_image[*IC].ARE = ARE_ABSOLUTE; /* Register addresses are absolute */
        (*IC)++;
        break;
    }

    case DIRECT_ADDR: /* Label - direct addressing */
    {
        /* Look up the symbol in the symbol table */
        symbol *sym = find_symbol(table, arg);

        if (sym)
        {
            /* Symbol found - encode its address */
            code_image[*IC].code = sym->address;

            if (sym->type == SYMBOL_EXTERN)
            {
                /* External symbols: address is 0, marked as External */
                char IC_str[MAX_LINE_LEN];
                code_image[*IC].ARE = ARE_EXTERNAL;
                code_image[*IC].code = 0;

                /* Add this reference to the externals list for the .ext file */
                sprintf(IC_str, "%d", *IC);
                if (ext_list == NULL)
                    ext_list = create_node(arg, IC_str);
                else
                    add_node(ext_list, create_node(arg, IC_str));
            }
            /* Internal symbols are marked as relocatable */
            else
                code_image[*IC].ARE = ARE_RELOCATABLE;
        }
        /* Symbol not found in table - undefined label error */
        else
            log_error(ERR_UNDEFINED_LABEL, line_number, NULL, arg);
        (*IC)++;
        break;
    }

    case INDEX_ADDR: /* %Label - relative/index addressing */
    {
        /* Look up the symbol (skip the '%' prefix) */
        symbol *sym = find_symbol(table, arg + 1);

        if (sym)
        {
            /* External symbols cannot use relative addressing */
            if (sym->type == SYMBOL_EXTERN)
                log_error(ERR_INVALID_OPERAND_TYPE, line_number, NULL,
                          "Relative addressing cannot be external");
            else
            {
                /* Calculate the distance from current IC to the symbol's address */
                int diff = sym->address - *IC;
                /* Store the relative offset */
                code_image[*IC].code = diff;
                code_image[*IC].ARE = ARE_ABSOLUTE; /* Offsets are absolute values */
                (*IC)++;
            }
        }
        /* Symbol not found */
        else
            log_error(ERR_INVALID_OPERAND_TYPE, line_number, NULL, arg);
        break;
    }
    default:
        break;
    }
}

/**
 * Function: encode_instruction
 * Description: Parses an instruction line and encodes it into the code image.
 * @param line - The instruction line to encode
 * @param table - The symbol table for resolving labels
 * @param IC - Pointer to the Instruction Counter
 * @param line_num - Current line number for error reporting
 * @return TRUE if successful, FALSE otherwise
 */
boolean encode_instruction(char *line, SymbolTable *table, int *IC, int line_num)
{
    char op_name[MAX_LINE_LEN], arg1[MAX_LINE_LEN] = {0}, arg2[MAX_LINE_LEN] = {0};
    char *ptr = line;
    unsigned int word;
    int src_mode = 0, dest_mode = 0;
    const CmdInfo *cmd;

    /* Extract the operation name */
    get_next_word(&ptr, op_name, FALSE);

    /* Look up the command information */
    cmd = find_cmd_info(op_name);
    if (cmd == NULL)
        return log_error(ERR_UNKNOWN_INSTRUCTION, line_num, NULL, op_name);

    /* Extract operands based on how many the instruction requires */
    if (cmd->op_count >= 1)
        get_next_word(&ptr, arg1, TRUE);
    if (cmd->op_count == 2)
        get_next_word(&ptr, arg2, TRUE);

    /* Determine the addressing modes for the operands */
    if (cmd->op_count == 2)
    {
        /* Two operands: first is source, second is destination */
        src_mode = get_arg_mode(arg1);
        dest_mode = get_arg_mode(arg2);
    }
    else if (cmd->op_count == 1)
    {
        /* One operand: it's the destination, source mode is 0 */
        src_mode = 0;
        dest_mode = get_arg_mode(arg1);
    }

    /* Build and store the first word of the instruction */
    word = build_word(cmd->opcode, cmd->funct, src_mode, dest_mode);
    code_image[*IC].code = word;
    code_image[*IC].ARE = ARE_ABSOLUTE; /* The instruction word itself is absolute */
    (*IC)++;

    /* Encode additional words for the operands */
    if (cmd->op_count == 2)
    {
        /* Encode both source and destination operands */
        encode_operand(arg1, table, src_mode, IC, line_num);
        encode_operand(arg2, table, dest_mode, IC, line_num);
    }

    /* Encode only the destination operand */
    else if (cmd->op_count == 1)
        encode_operand(arg1, table, dest_mode, IC, line_num);
    return TRUE;
}

/**
 * Function: write_object_file
 * Description: Writes the object file containing the encoded machine code and data.
 * @param filename - The base filename (without extension)
 * @param ICF - Final Instruction Counter value
 * @param DCF - Final Data Counter value
 */
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

    /* Write header line: instruction count and data count */
    fprintf(obj_file, "\t%d %d\n", ICF - IC_INIT_VALUE, DCF);

    /* Write the code section */
    for (i = IC_INIT_VALUE; i < ICF; i++)
    {
        int ARE = code_image[i].ARE;
        /* Convert ARE to a character: A=Absolute, E=External, R=Relocatable */
        char ARE_char = ARE == ARE_ABSOLUTE ? 'A' : ARE == ARE_EXTERNAL ? 'E'
                                                                        : 'R';
        /* Format: address, 12-bit hex value, ARE flag */
        fprintf(obj_file, "%04d %03X %c\n", i, code_image[i].code & 0xFFF, ARE_char);
    }

    /* Write the data section (data is placed after code in memory) */
    for (i = 0; i < DCF; i++)
        fprintf(obj_file, "%04d %03X A\n", ICF + i, data_image[i].code & 0xFFF); /* Data always has 'A' (Absolute) flag */

    /* Cleanup */
    fclose(obj_file);
    free(obj_filename);
}

/**
 * Function: write_entries_file
 * Description: Writes the entries file containing all entry point symbols.
 * @param filename - The base filename (without extension)
 * @param table - The symbol table containing entry symbols
 */
void write_entries_file(char *filename, SymbolTable *table)
{
    /* Only create the file if there are entry symbols */
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
        /* Write each entry symbol with its address */
        for (i = 0; i < table->count; i++)
        {
            if (table->symbols[i].is_entry)
                fprintf(ent_file, "%s %04d\n", table->symbols[i].name, table->symbols[i].address);
        }
        fclose(ent_file);
        free(ent_filename);
    }
}

/**
 * Function: write_externals_file
 * Description: Writes the externals file containing all external symbol references.
 * @param filename - The base filename (without extension)
 * @param table - The symbol table (not directly used, but kept for consistency)
 */
void write_externals_file(char *filename, SymbolTable *table)
{
    /* Only create the file if there are external references */
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
        /* Write each external reference with the address where it's used */
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

/**
 * Function: run_second_pass
 * Description: Main controller for the second pass of the assembler.
 * @param filename - The base filename to process
 * @param table - The symbol table built during the first pass
 * @param ICF - Final Instruction Counter from first pass
 * @param DCF - Final Data Counter from first pass
 * @return TRUE if successful, FALSE otherwise
 */
int run_second_pass(char *filename, SymbolTable *table, int ICF, int DCF)
{
    int IC = IC_INIT_VALUE, DC = 0;
    char line[MAX_LINE_LEN], curr_word[MAX_LINE_LEN];
    int line_number = 0;
    char *am_filename = create_file_path("output", filename, "am");
    FILE *am_file = fopen(am_filename, "r");

    /* Initialize the code and data images */
    memset(code_image, 0, sizeof(code_image));
    memset(data_image, 0, sizeof(data_image));
    ext_list = NULL;

    if (am_file == NULL)
    {
        log_error(ERR_OPEN_FILE, 0, NULL, am_filename);
        free(am_filename);
        return FALSE;
    }

    /* Process the file line by line */
    while (fgets(line, MAX_LINE_LEN, am_file))
    {
        char *ptr = skip_whitespaces(line);
        char first_word[MAX_LINE_LEN] = {0};
        line_number++;

        /* Extract the first word, skipping any label */
        get_first_word(ptr, first_word);

        /* If the first word is a label, skip it */
        if (first_word[strlen(first_word) - 1] == ':')
            get_next_word(&ptr, first_word, FALSE);

        /* Skip empty lines */
        ptr = skip_whitespaces(ptr);
        if (*ptr == '\0')
            continue;

        /* Handle directives (lines starting with '.') */
        if (*ptr == '.')
        {
            get_next_word(&ptr, curr_word, FALSE);

            /* Process .entry directive */
            if (strcmp(curr_word, ".entry") == 0)
            {
                char label[MAX_LABEL_LEN];
                symbol *sym;

                /* Extract the label name */
                get_next_word(&ptr, label, FALSE);

                sym = find_symbol(table, label);
                /* Verify that the label exists in the symbol table */
                if (sym == NULL)
                {
                    log_error(ERR_UNDEFINED_LABEL, line_number, am_filename, label);
                    continue;
                }

                /* checks for collision with .extren */
                if (sym->type == SYMBOL_EXTERN)
                {
                    log_error(ERR_INVALID_OPERAND_TYPE, line_number, am_filename, "Label cannot be both entry and extern");
                    continue;
                }
                /* Mark the symbol as an entry point */
                update_symbol(table, label, -1, TRUE);
            }
            /* Process .data and .string directives */
            if (strcmp(curr_word, ".data") == 0 || strcmp(curr_word, ".string") == 0)
                encode_data_lines(ptr, curr_word, &DC, line_number);
        }

        /* This is an instruction line - encode it */
        else
            encode_instruction(ptr, table, &IC, line_number);
    }

    /* Generate all output files */
    if (!was_error_found)
    {
        write_object_file(filename, ICF, DCF);
        write_entries_file(filename, table);
        write_externals_file(filename, table);
    }

    /* Clean up */
    fclose(am_file);
    free_nodes(ext_list);
    ext_list = NULL;
    free(am_filename);

    return TRUE;
}