#include <stdio.h>
#include <stdlib.h>
#include "./opcodes.h"

#define NUM_REGISTERS 8
#define MEMORY_SIZE 1024

int registers[NUM_REGISTERS];
unsigned char memory[MEMORY_SIZE];

void load_program();
void execute();

int main() {
    load_program();
    execute();

    // Exibir o estado final dos registradores
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("R%d: %d\n", i, registers[i]);
    }
    return 0;
}

void load_program() {
    // MOV R0, 10
    memory[0] = OPCODE_MOV;
    memory[1] = 0x00; // R0
    memory[2] = 10;

    // MOV R1, 20
    memory[3] = OPCODE_MOV;
    memory[4] = 0x01; // R1
    memory[5] = 20;

    // ADD R0, R1
    memory[6] = OPCODE_ADD;
    memory[7] = 0x00; // R0
    memory[8] = 0x01; // R1

    // HLT
    memory[9] = OPCODE_HLT;
}

void execute() {
    int pc = 0; // Program Counter

    while (1) {
        // Buscar a instrução
        unsigned char opcode = memory[pc++];
        unsigned char operand1 = memory[pc++];
        unsigned char operand2 = memory[pc++];

        // Decodificar e executar
        switch (opcode) {
            case OPCODE_MOV:
                registers[operand1] = operand2;
                break;
            case OPCODE_ADD:
                registers[operand1] += registers[operand2];
                break;
            case OPCODE_SUB:
                registers[operand1] -= registers[operand2];
                break;
            case OPCODE_JMP:
                pc = operand1;
                break;
            case OPCODE_HLT:
                return; // Encerrar execução
            default:
                printf("Instrução desconhecida: 0x%X\n", opcode);
                return;
        }
    }
}
