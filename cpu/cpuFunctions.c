#include "cpu.h"
#include "../opcodes.h"

#include <string.h>

void cpu_reset(Cpu *cpu) {
    memset(cpu, 0, sizeof(*cpu));
    cpu->status = CPU_RUNNING;
}

CpuStatus cpu_load(Cpu *cpu, const uint8_t *program, size_t size) {
    cpu_reset(cpu);
    if (program == NULL || size == 0 || size > MEMORY_SIZE || size % INSTRUCTION_SIZE != 0) {
        cpu->status = CPU_BAD_PROGRAM;
        return cpu->status;
    }
    memcpy(cpu->memory, program, size);
    cpu->program_size = size;
    return cpu->status;
}

CpuStatus cpu_step(Cpu *cpu) {
    if (cpu->status != CPU_RUNNING) {
        return cpu->status;
    }
    if (cpu->pc >= cpu->program_size || cpu->pc % INSTRUCTION_SIZE != 0 ||
        cpu->program_size - cpu->pc < INSTRUCTION_SIZE) {
        cpu->status = CPU_BAD_PC;
        return cpu->status;
    }

    const uint8_t opcode = cpu->memory[cpu->pc];
    const uint8_t arg1 = cpu->memory[cpu->pc + 1];
    const uint8_t arg2 = cpu->memory[cpu->pc + 2];

    switch (opcode) {
        case OPCODE_MOV:
            if (arg1 >= NUM_REGISTERS) {
                cpu->status = CPU_BAD_REGISTER;
                return cpu->status;
            }
            cpu->registers[arg1] = arg2;
            break;
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_XOR:
            if (arg1 >= NUM_REGISTERS || arg2 >= NUM_REGISTERS) {
                cpu->status = CPU_BAD_REGISTER;
                return cpu->status;
            }
            if (opcode == OPCODE_ADD) {
                cpu->registers[arg1] = (uint8_t)(cpu->registers[arg1] + cpu->registers[arg2]);
            } else if (opcode == OPCODE_SUB) {
                cpu->registers[arg1] = (uint8_t)(cpu->registers[arg1] - cpu->registers[arg2]);
            } else {
                cpu->registers[arg1] ^= cpu->registers[arg2];
            }
            break;
        case OPCODE_JMP: {
            const uint16_t target = (uint16_t)((uint16_t)arg1 | ((uint16_t)arg2 << 8));
            if (target >= cpu->program_size || target % INSTRUCTION_SIZE != 0) {
                cpu->status = CPU_BAD_JUMP;
                return cpu->status;
            }
            cpu->pc = target;
            return cpu->status;
        }
        case OPCODE_HLT:
            if (arg1 != 0 || arg2 != 0) {
                cpu->status = CPU_BAD_ENCODING;
                return cpu->status;
            }
            cpu->pc += INSTRUCTION_SIZE;
            cpu->status = CPU_HALTED;
            return cpu->status;
        default:
            cpu->status = CPU_BAD_OPCODE;
            return cpu->status;
    }

    cpu->pc += INSTRUCTION_SIZE;
    return cpu->status;
}

const char *cpu_status_message(CpuStatus status) {
    switch (status) {
        case CPU_RUNNING: return "running";
        case CPU_HALTED: return "halted";
        case CPU_BAD_PROGRAM: return "empty, oversized or incomplete bytecode";
        case CPU_BAD_PC: return "PC outside loaded instructions (missing HLT?)";
        case CPU_BAD_OPCODE: return "unknown opcode";
        case CPU_BAD_REGISTER: return "invalid register index";
        case CPU_BAD_JUMP: return "jump target outside the program or not aligned to an instruction";
        case CPU_BAD_ENCODING: return "non-zero HLT padding";
        default: return "unknown CPU status";
    }
}
