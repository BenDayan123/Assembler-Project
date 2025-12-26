CC = gcc
CFLAGS = -Wall -ansi -pedantic

SRC_DIR = src
OUT_DIR = output
BUILD_DIR = build

OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/utils.o
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f $(BUILD_DIR)