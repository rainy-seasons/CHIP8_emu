# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude

# Source and build setup
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
BIN = $(BIN_DIR)/chip8.exe

# Collect all .c files in src/
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default target
all: $(BIN)

# Link executable
$(BIN): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -Llib -lSDL3 -o $@

# Compile each .c into .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create obj and bin directories if missing
$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

