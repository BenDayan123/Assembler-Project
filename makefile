CC = gcc
CFLAGS = -Wall -ansi -pedantic

SRC = src
BUILD_DIR = build
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/utils.o
TARGET = main

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(SRC)/$(TARGET)

clean:
	rm -f $(BUILD_DIR)