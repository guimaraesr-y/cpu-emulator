#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu/cpu.h"

#define MAX_STEPS 100000u

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program.bin>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror("Cannot open bytecode");
        return EXIT_FAILURE;
    }

    uint8_t bytecode[MEMORY_SIZE + 1];
    const size_t size = fread(bytecode, 1, sizeof(bytecode), input);
    if (ferror(input)) {
        perror("Cannot read bytecode");
        fclose(input);
        return EXIT_FAILURE;
    }
    if (fclose(input) != 0) {
        perror("Cannot close bytecode");
        return EXIT_FAILURE;
    }

    Cpu cpu;
    if (cpu_load(&cpu, bytecode, size) != CPU_RUNNING) {
        fprintf(stderr, "Invalid program: %s\n", cpu_status_message(cpu.status));
        return EXIT_FAILURE;
    }

    unsigned steps = 0;
    while (cpu.status == CPU_RUNNING && steps < MAX_STEPS) {
        cpu_step(&cpu);
        steps++;
    }
    if (cpu.status == CPU_RUNNING) {
        fprintf(stderr, "Execution error: step limit reached (%u)\n", MAX_STEPS);
        return EXIT_FAILURE;
    }
    if (cpu.status != CPU_HALTED) {
        fprintf(stderr, "Execution error at PC=%u: %s\n",
                (unsigned)cpu.pc, cpu_status_message(cpu.status));
        return EXIT_FAILURE;
    }

    printf("HLT after %u instruction(s)\nFinal State of Registers:\n", steps);
    for (unsigned i = 0; i < NUM_REGISTERS; i++) {
        printf("R%u: %u\n", i, (unsigned)cpu.registers[i]);
    }
    return EXIT_SUCCESS;
}
