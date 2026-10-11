#include "playback.h"
#include "../opcodes.h"
#include <stdio.h>
#include <string.h>

CpuStatus lab_load(Playback *lab, const uint8_t *program, size_t size) {
    memset(lab, 0, sizeof(*lab));
    CpuStatus status = cpu_load(&lab->cpu, program, size);
    if (status == CPU_RUNNING) {
        memcpy(lab->program, program, size);
        lab->size = size;
    }
    return status;
}

void lab_reset(Playback *lab) {
    cpu_load(&lab->cpu, lab->program, lab->size);
    memset(&lab->trace, 0, sizeof(lab->trace));
    lab->phase = LAB_IDLE;
    lab->elapsed = 0;
    lab->steps = 0;
}

bool lab_step(Playback *lab) {
    if (lab->phase != LAB_IDLE || lab->cpu.status != CPU_RUNNING) return false;
    CpuTrace *t = &lab->trace;
    memset(t, 0, sizeof(*t));
    t->before = lab->cpu;
    size_t pc = lab->cpu.pc;
    t->fetched = pc < lab->cpu.program_size && pc % INSTRUCTION_SIZE == 0 &&
        lab->cpu.program_size - pc >= INSTRUCTION_SIZE;
    if (t->fetched) memcpy(t->bytes, lab->cpu.memory + pc, INSTRUCTION_SIZE);
    cpu_step(&lab->cpu);
    t->after = lab->cpu;
    for (unsigned i = 0; i < NUM_REGISTERS; ++i)
        if (t->before.registers[i] != t->after.registers[i]) t->changed |= 1u << i;
    lab->phase = LAB_FETCH;
    lab->elapsed = 0;
    ++lab->steps;
    return true;
}

void lab_update(Playback *lab, float seconds) {
    const float duration = 0.55f;
    if (seconds <= 0 || lab->phase == LAB_IDLE) return;
    lab->elapsed += seconds;
    while (lab->elapsed >= duration && lab->phase != LAB_IDLE) {
        lab->elapsed -= duration;
        lab->phase = lab->phase == LAB_WRITEBACK ? LAB_IDLE : (LabPhase)(lab->phase + 1);
    }
    if (lab->phase == LAB_IDLE) lab->elapsed = 0;
}

const Cpu *lab_visible(const Playback *lab) {
    return lab->phase != LAB_IDLE && lab->phase != LAB_WRITEBACK ?
        &lab->trace.before : &lab->cpu;
}

const char *lab_phase_name(LabPhase phase) {
    static const char *names[] = {"READY", "FETCH", "DECODE", "EXECUTE", "WRITEBACK"};
    return names[phase];
}

void lab_instruction(const CpuTrace *t, char *text, size_t capacity) {
    unsigned a = t->bytes[1], b = t->bytes[2];
    if (!t->fetched) { snprintf(text, capacity, "No instruction fetched"); return; }
    switch (t->bytes[0]) {
        case OPCODE_MOV: snprintf(text, capacity, "MOV R%u, %u", a, b); break;
        case OPCODE_ADD: snprintf(text, capacity, "ADD R%u, R%u", a, b); break;
        case OPCODE_SUB: snprintf(text, capacity, "SUB R%u, R%u", a, b); break;
        case OPCODE_XOR: snprintf(text, capacity, "XOR R%u, R%u", a, b); break;
        case OPCODE_JMP: snprintf(text, capacity, "JMP %u", a | (b << 8)); break;
        case OPCODE_HLT: snprintf(text, capacity, "HLT"); break;
        default: snprintf(text, capacity, "Unknown opcode 0x%02X", t->bytes[0]); break;
    }
}

void lab_explanation(const Playback *lab, char *text, size_t capacity) {
    const CpuTrace *t = &lab->trace;
    if (lab->steps == 0) {
        snprintf(text, capacity, "%s", lab->cpu.status == CPU_RUNNING ?
            "Press STEP to follow one real instruction." : cpu_status_message(lab->cpu.status)); return;
    }
    if (t->after.status != CPU_RUNNING && t->after.status != CPU_HALTED) {
        snprintf(text, capacity, "Fault at byte %u: %s", t->before.pc,
                 cpu_status_message(t->after.status)); return;
    }
    unsigned a = t->bytes[1], b = t->bytes[2];
    switch (lab->phase) {
        case LAB_FETCH:
            snprintf(text, capacity, "Read bytes %u, %u, %u from RAM.",
                     t->before.pc, t->before.pc + 1, t->before.pc + 2); return;
        case LAB_DECODE:
            snprintf(text, capacity, "Decode opcode 0x%02X and its two operand bytes.", t->bytes[0]); return;
        default: break;
    }
    switch (t->bytes[0]) {
        case OPCODE_MOV:
            snprintf(text, capacity, "Immediate %u enters R%u. Result: %u.", b, a, t->after.registers[a]); break;
        case OPCODE_ADD: case OPCODE_SUB: case OPCODE_XOR:
            snprintf(text, capacity, "ALU inputs %u and %u. R%u result: %u (8 bits).",
                t->before.registers[a], t->before.registers[b], a, t->after.registers[a]); break;
        case OPCODE_JMP:
            snprintf(text, capacity, "Control redirects PC: byte %u -> byte %u.", t->before.pc, t->after.pc); break;
        case OPCODE_HLT:
            snprintf(text, capacity, "CPU halted. Final PC: %u. Reset to run again.", t->after.pc); break;
        default: snprintf(text, capacity, "%s", cpu_status_message(t->after.status)); break;
    }
}
