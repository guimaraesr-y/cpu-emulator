#include <stdio.h>
#include "../variables.h"

void readBytesFromMemory(int* pc, unsigned char* memory, unsigned char* arg1, unsigned char* arg2) {
    *arg1 = memory[(*pc)++];

    if (arg2 != NULL) {
        *arg2 = memory[(*pc)++];
    }
}

void execute_mov(int *pc, int *registers, unsigned char *memory) {
    unsigned char reg;
    unsigned char value;
    readBytesFromMemory(pc, memory, &reg, &value);

    if (reg >= NUM_REGISTERS) {
        printf("Error: Invalid register R%d\n", reg);
        return;
    }
    registers[reg] = value;
    printf("MOV R%d, %d\n", reg, value);
}

void execute_add(int *pc, int *registers, unsigned char *memory) {
    unsigned char reg1;
    unsigned char reg2;
    readBytesFromMemory(pc, memory, &reg1, &reg2);

    if (reg1 >= NUM_REGISTERS || reg2 >= NUM_REGISTERS) {
        printf("Error: Invalid register(s) in ADD\n");
        return;
    }
    registers[reg1] += registers[reg2];
    printf("ADD R%d, R%d\n", reg1, reg2);
}

void execute_sub(int *pc, int *registers, unsigned char *memory) {
    unsigned char reg1;
    unsigned char reg2;
    readBytesFromMemory(pc, memory, &reg1, &reg2);

    if (reg1 >= NUM_REGISTERS || reg2 >= NUM_REGISTERS) {
        printf("Error: Invalid register(s) in SUB\n");
        return;
    }
    registers[reg1] -= registers[reg2];
    printf("SUB R%d, R%d\n", reg1, reg2);
}

void execute_jmp(int *pc, unsigned char *memory) {
    unsigned char address;
    readBytesFromMemory(pc, memory, &address, NULL);

    int addressInt = (int)address;
    if (addressInt >= MEMORY_SIZE) {
        printf("Error: Invalid jump address %d\n", addressInt);
        return;
    }
    *pc = addressInt;
    printf("JMP %d\n", addressInt);
}

void execute_xor(int *pc, int *registers, unsigned char *memory) {
    unsigned char reg1;
    unsigned char reg2;
    readBytesFromMemory(pc, memory, &reg1, &reg2);

    if (reg1 >= NUM_REGISTERS || reg2 >= NUM_REGISTERS) {
        printf("Error: Invalid register(s) in XOR\n");
        return;
    }
    registers[reg1] ^= registers[reg2];
    printf("XOR R%d, R%d\n", reg1, reg2);
}
