#include <stdio.h>
#include <stdlib.h>
#include "./opcodes.h"

#define NUM_REGISTERS 8
#define MEMORY_SIZE 1024

void execute_mov(int *pc);
void execute_add(int *pc);
void execute_sub(int *pc);
void execute_jmp(int *pc);
void execute_xor(int *pc);  

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
                execute_mov(&pc);
                break;
            case OPCODE_ADD:
                execute_add(&pc);
                break;
            case OPCODE_SUB:
                execute_sub(&pc);
                break;
            case OPCODE_JMP:
                execute_jmp(&pc);
                break;
            case OPCODE_XOR:
                execute_xor(&pc);
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

void execute_mov(int *pc) {
    unsigned char reg = memory[(*pc)++];
    unsigned char value = memory[(*pc)++];
    if (reg >= NUM_REGISTERS) {
        printf("Error: Invalid register R%d\n", reg);
        return;
    }
    registers[reg] = value;
    printf("MOV R%d, %d\n", reg, value);
}

void execute_add(int *pc) {
    unsigned char reg1 = memory[(*pc)++];
    unsigned char reg2 = memory[(*pc)++];
    if (reg1 >= NUM_REGISTERS || reg2 >= NUM_REGISTERS) {
        printf("Error: Invalid register(s) in ADD\n");
        return;
    }
    registers[reg1] += registers[reg2];
    printf("ADD R%d, R%d\n", reg1, reg2);
}

void execute_sub(int *pc) {
    unsigned char reg1 = memory[(*pc)++];
    unsigned char reg2 = memory[(*pc)++];
    if (reg1 >= NUM_REGISTERS || reg2 >= NUM_REGISTERS) {
        printf("Error: Invalid register(s) in SUB\n");
        return;
    }
    registers[reg1] -= registers[reg2];
    printf("SUB R%d, R%d\n", reg1, reg2);
}

void execute_jmp(int *pc) {
    int address = memory[(*pc)++];
    if (address >= MEMORY_SIZE) {
        printf("Error: Invalid jump address %d\n", address);
        return;
    }
    *pc = address;
    printf("JMP %d\n", address);
}

void execute_xor(int *pc) {
    unsigned char reg1 = memory[(*pc)++];
    unsigned char reg2 = memory[(*pc)++];
    if (reg1 >= NUM_REGISTERS || reg2 >= NUM_REGISTERS) {
        printf("Error: Invalid register(s) in XOR\n");
        return;
    }
    registers[reg1] ^= registers[reg2];
    printf("XOR R%d, R%d\n", reg1, reg2);
}
