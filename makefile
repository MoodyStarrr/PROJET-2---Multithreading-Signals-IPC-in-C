CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -Iinclude -std=c99
LDLIBS = -pthread
SRC = $(wildcard src/*.c)
OBJ = $(SRC:source/%.c=build/%.o)
BIN = projet_2_app

.PHONY: all run re clean gdb valgrind

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)
	@# $^ pour appeler tous les .o présents dans build/

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $^ -o $@
	@# $^ pour appeler tous les .c présents dans source/

run: $(BIN)
	@./$(BIN)
re: clean all

clean:
	@$(RM) -r $(BIN) build logs/*.txt

gdb: $(BIN)
	gdb ./$<

valgrind: $(BIN)
	valgrind -s --leak-check=full --show-leak-kinds=all ./$<

