# =========================================
# Assembler Project - Makefile
# =========================================

# Compiler and Flags
CC = gcc
CFLAGS = -Wall -ansi -pedantic

# Directories
SRC_DIR = src
OUT_DIR = output
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c) # This automatically finds all .c files in /src directory
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -rf $(wildcard $(BUILD_DIR)/*.o) $(TARGET)

clean_output:
	rm -rf $(wildcard $(OUT_DIR)/*.am)