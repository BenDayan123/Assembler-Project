#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "../headers/globals.h"

const CmdInfo *find_cmd_info(char *cmd);
addressing_mode get_arg_mode(char *arg);
boolean is_vaild_command_line(char *cmd_name, char *args, int line, char *filename);
char *create_file_path(const char *path, char *filename, const char *ext);
char *skip_whitespaces(char *str);
void *handle_malloc(size_t size);
void get_first_word(const char *line, char *word);
int get_next_word(char **src, char *dest, boolean expect_comma);
int count_and_validate_data_numbers(char *line);
int is_register(const char *op);
boolean is_valid_addressing(int opcode, int op_count, addressing_mode src, addressing_mode dest);

#endif