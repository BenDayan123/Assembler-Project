#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

char *convert_to_binary(unsigned int num, unsigned int binary_length);
char *filename_with_ext(char *filename, const char *ext);
char *skip_whitespaces(char *str);
char *handle_malloc(size_t size);

#endif