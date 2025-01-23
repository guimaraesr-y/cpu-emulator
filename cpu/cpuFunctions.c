#include "../variables.h"

void execute_mov(int *pc, int *registers, unsigned char *memory) {
    unsigned char reg = memory[(*pc)++];
    unsigned char value = memory[(*pc)++];
    if (reg >= NUM_REGISTERS) {
        printf("Error: Invalid register R%d\n", reg);
        return;
    }
    registers[reg] = value;
    printf("MOV R%d, %d\n", reg, value);
}

void execute_add(int *pc, int *registers, unsigned char *memory) {
    unsigned char reg1 = memory[(*pc)++];
    unsigned char reg2 = memory[(*pc)++];
    if (reg1 >= NUM_REGISTERS || reg2 >= NUM_REGISTERS) {
        printf("Error: Invalid register(s) in ADD\n");
        return;
    }
    registers[reg1] += registers[reg2];
    printf("ADD R%d, R%d\n", reg1, reg2);
}

void execute_sub(int *pc, int *registers, unsigned char *memory) {
    unsigned char reg1 = memory[(*pc)++];
    unsigned char reg2 = memory[(*pc)++];
    if (reg1 >= NUM_REGISTERS || reg2 >= NUM_REGISTERS) {
        printf("Error: Invalid register(s) in SUB\n");
        return;
    }
    registers[reg1] -= registers[reg2];
    printf("SUB R%d, R%d\n", reg1, reg2);
}

void execute_jmp(int *pc, unsigned char *memory) {
    int address = memory[(*pc)++];
    if (address >= MEMORY_SIZE) {
        printf("Error: Invalid jump address %d\n", address);
        return;
    }
    *pc = address;
    printf("JMP %d\n", address);
}

void execute_xor(int *pc, int *registers, unsigned char *memory) {
    unsigned char reg1 = memory[(*pc)++];
    unsigned char reg2 = memory[(*pc)++];
    if (reg1 >= NUM_REGISTERS || reg2 >= NUM_REGISTERS) {
        printf("Error: Invalid register(s) in XOR\n");
        return;
    }
    registers[reg1] ^= registers[reg2];
    printf("XOR R%d, R%d\n", reg1, reg2);
}
