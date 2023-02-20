CC=gcc
CFLAGS=-Wall -Wextra -Wswitch-enum -pedantic -std=c11 -ggdb

SRC=src
BIN=exe
BUILD=build

SRC_FILES=$(wildcard $(SRC)/*.c)
OBJ_FILES=$(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(SRC_FILES))
EXECUTABLE=ruja

.DEFAULT_GOAL := build

memcheck: dirs build
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s -q $(BIN)/$(EXECUTABLE) input.ruja

build: dirs $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) ruja.c -o $(BIN)/$(EXECUTABLE)

$(BUILD)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

dirs:
	mkdir -p $(BIN) $(BUILD)

clean:
	rm -f $(BUILD)/*.o $(BIN)/$(EXECUTABLE)