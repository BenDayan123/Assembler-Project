#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "../headers/globals.h"

char *convert_to_binary(unsigned int num, unsigned int binary_length);
CmdInfo *find_cmd_info(char *cmd);
boolean is_vaild_command_line(char *cmd_name, char *args, int line, char *filename);
char *filename_with_ext(char *filename, const char *ext);
char *join_path_and_ext(char *filename, const char *ext, const char *path);
char *skip_whitespaces(char *str);
void *handle_malloc(size_t size);
void get_first_word(const char *line, char *word);
int get_next_word(char **src, char *dest, boolean expect_comma);
int count_and_validate_data_numbers(char *line);
int is_register(const char *op);

#endif