#ifndef LAB_ASSEMBLER_H
#define LAB_ASSEMBLER_H
#include "../variables.h"
#include <stdint.h>
#include <stddef.h>
typedef struct {
    uint8_t bytes[MEMORY_SIZE];
    size_t size;
    size_t lines[MEMORY_SIZE/INSTRUCTION_SIZE];
    size_t error_line;
    char error[256];
} AsmResult;
int assemble_text(const char *source, AsmResult *result);
#endif
