TARGET = uHDLC
BUILD = build
CC = gcc
SRC = src/main.c src/uHDLC.c
FLAGS = -Wall -Wextra -Werror -pedantic -fstack-protector-strong

$(TARGET): $(SRC)
	@$(CC) $(FLAGS) -o $(BUILD)/$(TARGET) $(SRC)
	@chmod +x $(BUILD)/$(TARGET)
	@./$(BUILD)/$(TARGET)