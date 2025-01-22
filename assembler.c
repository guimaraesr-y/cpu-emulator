#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "../opcodes.h"

void assembleLine(char* line, FILE* outputFile);
int getOpcode(char* opcodeString);

void mov(int r, int value, FILE* outputFile);
void add(int r1, int r2, FILE* outputFile); 
void sub(int r1, int r2, FILE* outputFile);
void jmp(int address, FILE* outputFile);
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
            // No operands needed for HLT
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
            operand1 = atoi(operand1Str); // Convert to integer
        }
    }
    if (strlen(operand2Str) > 0) {
        if (operand2Str[0] == 'R') {
            operand2 = operand2Str[1] - '0'; // Convert 'R0' to 0, etc.
        } else {
            operand2 = atoi(operand2Str); // Convert to integer
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
    if (strcmp(opcodeString, "jmp") == 0) return OPCODE_JMP;
    if (strcmp(opcodeString, "hlt") == 0) return OPCODE_HLT;
    return -1; // Opcode desconhecido
}

void mov(int r, int value, FILE* outputFile) {
    unsigned char bytecode[3];
    bytecode[0] = OPCODE_MOV;
    bytecode[1] = r;
    bytecode[2] = value;
    printf("Opcode: %X R%d, %d\n", OPCODE_MOV, r, value);  
    fwrite(bytecode, sizeof(unsigned char), 3, outputFile);
}

void add(int r1, int r2, FILE* outputFile) {
    unsigned char bytecode[3];
    bytecode[0] = OPCODE_ADD;
    bytecode[1] = r1;
    bytecode[2] = r2;
    printf("Opcode: %X R%d, R%d\n", OPCODE_ADD, r1, r2);    
    fwrite(bytecode, sizeof(unsigned char), 3, outputFile);
}

void sub(int r1, int r2, FILE* outputFile) {
    unsigned char bytecode[3];
    bytecode[0] = OPCODE_SUB;
    bytecode[1] = r1;
    bytecode[2] = r2;
    printf("Opcode: %X R%d, R%d\n", OPCODE_SUB, r1, r2);    
    fwrite(bytecode, sizeof(unsigned char), 3, outputFile);
}

void jmp(int address, FILE* outputFile) {
    unsigned char bytecode[2];
    bytecode[0] = OPCODE_JMP;
    bytecode[1] = address;
    printf("Opcode: %X %d\n", OPCODE_JMP, address);    
    fwrite(bytecode, sizeof(unsigned char), 2, outputFile);
}

void hlt(FILE* outputFile) {
    unsigned char bytecode[1];
    bytecode[0] = OPCODE_HLT;
    printf("Opcode: %X\n", OPCODE_HLT);    
    fwrite(bytecode, sizeof(unsigned char), 1, outputFile);
}