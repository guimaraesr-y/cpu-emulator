CC = cc
CFLAGS = -std=c99 -O2 -Wall -Wextra -Wpedantic

BIN_DIR = bin
ASSEMBLER = $(BIN_DIR)/assembler
EMULATOR = $(BIN_DIR)/emulator

.PHONY: all clean test
all: $(ASSEMBLER) $(EMULATOR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(ASSEMBLER): assembler.c opcodes.h variables.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ assembler.c

$(EMULATOR): main.c cpu/cpuFunctions.c cpu/cpu.h opcodes.h variables.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ main.c cpu/cpuFunctions.c

test: all
	python3 -m unittest discover -s tests -v

clean:
	rm -rf $(BIN_DIR)
