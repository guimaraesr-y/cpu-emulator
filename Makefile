# Makefile for CPU Emulator and Assembler

CC = gcc
CFLAGS = -Wall -Wextra -std=c99

ASSEMBLER = assembler
EMULATOR = emulator

ASSEMBLER_SRC = assembler.c
EMULATOR_SRC = main.c

HEADERS = opcodes.h

all: $(ASSEMBLER) $(EMULATOR)

$(ASSEMBLER): $(ASSEMBLER_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(ASSEMBLER) $(ASSEMBLER_SRC)

$(EMULATOR): $(EMULATOR_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(EMULATOR) $(EMULATOR_SRC)

clean:
	rm -f $(ASSEMBLER) $(EMULATOR)

.PHONY: all clean