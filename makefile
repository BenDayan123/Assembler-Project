CC = gcc
CFLAGS = -Wall -ansi -pedantic

BUILD_DIR = build
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/utils.o
TARGET = main

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -f $(BUILD_DIR)