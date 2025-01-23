#include <stdio.h>
#include <stdlib.h>
#include "./variables.h"
#include "./opcodes.h"
#include "./cpu/cpuFunctions.c"

int registers[NUM_REGISTERS];
unsigned char memory[MEMORY_SIZE];

void load_program(const char *filename);
void execute();

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Load the program into memory
    load_program(argv[1]);

    // Execute the program
    execute();

    // Display the final state of the registers
    printf("\nFinal State of Registers:\n");
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("R%d: %d\n", i, registers[i]);
    }

    return 0;
}

void load_program(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    size_t bytes_read = fread(memory, sizeof(unsigned char), MEMORY_SIZE, file);
    fclose(file);

    printf("Loaded %zu bytes into memory.\n", bytes_read);
}

void execute() {
    int pc = 0; // Program Counter

    printf("\nExecuting Program:\n");

    while (1) {
        if (pc >= MEMORY_SIZE) {
            printf("Error: Program counter out of memory bounds.\n");
            break;
        }

        // Fetch the instruction
        unsigned char opcode = memory[pc++];

        // Decode and execute the instruction
        switch (opcode) {
            case OPCODE_MOV:
                execute_mov(&pc, registers, memory);
                break;
            case OPCODE_ADD:
                execute_add(&pc, registers, memory);
                break;
            case OPCODE_SUB:
                execute_sub(&pc, registers, memory);
                break;
            case OPCODE_JMP:
                execute_jmp(&pc, memory);
                break;
            case OPCODE_XOR:
                execute_xor(&pc, registers, memory);
                break;
            case OPCODE_HLT:
                printf("HLT\n");
                return; // End execution
            default:
                printf("Error: Unknown instruction 0x%X at PC=%d\n", opcode, pc - 1);
                return;
        }
    }
}
