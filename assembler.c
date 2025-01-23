#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "./opcodes.h"
#include "./assembler/assemblerFunctions.c"

void assembleLine(char* line, FILE* outputFile);
int getOpcode(char* opcodeString);

void mov(int r, int value, FILE* outputFile);
void add(int r1, int r2, FILE* outputFile); 
void sub(int r1, int r2, FILE* outputFile);
void jmp(int address, FILE* outputFile);
void xor(int r1, int r2, FILE* outputFile);
void hlt(FILE* outputFile);

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    FILE* input_file = fopen(argv[1], "r");  
    if (input_file == NULL) {
        printf("Failed to open input file\n");      
        return 1;
    }

    FILE* output_file = fopen(argv[2], "wb");
    if (output_file == NULL) {
        printf("Failed to open output file\n");      
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), input_file) != NULL) {
        assembleLine(line, output_file);       
    }

    return 0;
}

void assembleLine(char* line, FILE* outputFile) {
    printf("Interpretando linha: %s", line);

    char opcodeString[4];
    sscanf(line, "%3s", opcodeString);

    int opcode = getOpcode(opcodeString);
    if (opcode == -1) {
        printf("Opcode desconhecido: %s\n", opcodeString);
        return;
    }

    int operand1 = 0, operand2 = 0;
    char operand1Str[8] = "", operand2Str[8] = "";

    // Parse operands based on the opcode
    switch (opcode) {
        case OPCODE_MOV:
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_XOR:
            if (sscanf(line, "%*s %7[^,], %7s", operand1Str, operand2Str) != 2) {
                printf("Erro: Operandos inválidos para %s\n", opcodeString);
                return;
            }
            break;
        case OPCODE_JMP:
            if (sscanf(line, "%*s %7s", operand1Str) != 1) {
                printf("Erro: Operando inválido para %s\n", opcodeString);
                return;
            }
            break;
        case OPCODE_HLT:
            break;
        default:
            printf("Opcode ainda não implementado: %s\n", opcodeString);
            return;
    }

    // Convert operands (if applicable)
    if (strlen(operand1Str) > 0) {
        if (operand1Str[0] == 'R') {
            operand1 = operand1Str[1] - '0'; // Convert 'R0' to 0, etc.
        } else {
            operand1 = atoi(operand1Str);
        }
    }
    if (strlen(operand2Str) > 0) {
        if (operand2Str[0] == 'R') {
            operand2 = operand2Str[1] - '0'; // Convert 'R0' to 0, etc.
        } else {
            operand2 = atoi(operand2Str);
        }
    }

    // Execute based on opcode
    switch (opcode) {
        case OPCODE_MOV:
            mov(operand1, operand2, outputFile);
            break;
        case OPCODE_ADD:
            add(operand1, operand2, outputFile);
            break;
        case OPCODE_SUB:
            sub(operand1, operand2, outputFile);
            break;
        case OPCODE_JMP:
            jmp(operand1, outputFile);
            break;
        case OPCODE_XOR:
            xor(operand1, operand2, outputFile);
            break;
        case OPCODE_HLT:
            hlt(outputFile);
            break;
        default:
            printf("Erro inesperado: Opcode desconhecido após validação.\n");
            break;
    }
}


int getOpcode(char* opcodeString) {
    if (strcmp(opcodeString, "mov") == 0) return OPCODE_MOV;
    if (strcmp(opcodeString, "add") == 0) return OPCODE_ADD;
    if (strcmp(opcodeString, "sub") == 0) return OPCODE_SUB;
    if (strcmp(opcodeString, "xor") == 0) return OPCODE_XOR;
    if (strcmp(opcodeString, "jmp") == 0) return OPCODE_JMP;
    if (strcmp(opcodeString, "hlt") == 0) return OPCODE_HLT;
    return -1; // Unknown Opcode
}
