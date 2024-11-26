TARGET = uHDLC.o
BUILD = build
CC = gcc
SRC = src/uHDLC.c
FLAGS = -Wall -Wextra -Werror -pedantic -fstack-protector-strong

$(TARGET): $(SRC)
	@$(CC) $(FLAGS) -c $(SRC) -o $(BUILD)/$(TARGET)
	@ar rcs $(BUILD)/libuHDLC.a $(BUILD)/$(TARGET)
	@rm $(BUILD)/$(TARGET)