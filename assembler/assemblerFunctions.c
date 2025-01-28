#include <stdio.h>
#include "../opcodes.h"

void mov(int r, int value, FILE* outputFile) {
    unsigned char bytecode[3];
    bytecode[0] = OPCODE_MOV;
    bytecode[1] = r;
    bytecode[2] = value;
    fwrite(bytecode, sizeof(unsigned char), LINE_SIZE, outputFile);
}

void add(int r1, int r2, FILE* outputFile) {
    unsigned char bytecode[3];
    bytecode[0] = OPCODE_ADD;
    bytecode[1] = r1;
    bytecode[2] = r2;
    fwrite(bytecode, sizeof(unsigned char), LINE_SIZE, outputFile);
}

void sub(int r1, int r2, FILE* outputFile) {
    unsigned char bytecode[3];
    bytecode[0] = OPCODE_SUB;
    bytecode[1] = r1;
    bytecode[2] = r2;
    fwrite(bytecode, sizeof(unsigned char), LINE_SIZE, outputFile);
}

void jmp(int address, FILE* outputFile) {
    unsigned char bytecode[2];
    bytecode[0] = OPCODE_JMP;
    bytecode[1] = address;
    bytecode[2] = 0x00;
    fwrite(bytecode, sizeof(unsigned char), LINE_SIZE, outputFile);
}

void xor(int r1, int r2, FILE* outputFile) {
    unsigned char bytecode[3];
    bytecode[0] = OPCODE_XOR;
    bytecode[1] = r1;
    bytecode[2] = r2;
    fwrite(bytecode, sizeof(unsigned char), LINE_SIZE, outputFile);
}

void hlt(FILE* outputFile) {
    unsigned char bytecode[1];
    bytecode[0] = OPCODE_HLT;
    bytecode[1] = OPCODE_HLT;
    bytecode[2] = OPCODE_HLT;
    fwrite(bytecode, sizeof(unsigned char), LINE_SIZE, outputFile);
}
