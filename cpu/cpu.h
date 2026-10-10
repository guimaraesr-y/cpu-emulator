#ifndef CPU_EMULATOR_CPU_H
#define CPU_EMULATOR_CPU_H

#include <stddef.h>
#include <stdint.h>
#include "../variables.h"

typedef enum {
    CPU_RUNNING = 0,
    CPU_HALTED,
    CPU_BAD_PROGRAM,
    CPU_BAD_PC,
    CPU_BAD_OPCODE,
    CPU_BAD_REGISTER,
    CPU_BAD_JUMP,
    CPU_BAD_ENCODING
} CpuStatus;

typedef struct {
    uint8_t memory[MEMORY_SIZE];
    uint8_t registers[NUM_REGISTERS];
    size_t program_size;
    uint16_t pc;
    CpuStatus status;
} Cpu;

void cpu_reset(Cpu *cpu);
CpuStatus cpu_load(Cpu *cpu, const uint8_t *program, size_t size);
CpuStatus cpu_step(Cpu *cpu);
const char *cpu_status_message(CpuStatus status);

#endif
