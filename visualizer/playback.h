#ifndef LAB_PLAYBACK_H
#define LAB_PLAYBACK_H
#include "../cpu/cpu.h"
#include <stdbool.h>

typedef enum { LAB_IDLE, LAB_FETCH, LAB_DECODE, LAB_EXECUTE, LAB_WRITEBACK } LabPhase;
typedef struct {
    Cpu before, after;
    uint8_t bytes[INSTRUCTION_SIZE];
    bool fetched;
    unsigned changed;
} CpuTrace;
typedef struct {
    Cpu cpu;
    CpuTrace trace;
    uint8_t program[MEMORY_SIZE];
    size_t size;
    LabPhase phase;
    float elapsed;
    unsigned steps;
} Playback;

CpuStatus lab_load(Playback *lab, const uint8_t *program, size_t size);
void lab_reset(Playback *lab);
bool lab_step(Playback *lab);
void lab_update(Playback *lab, float seconds);
const Cpu *lab_visible(const Playback *lab);
const char *lab_phase_name(LabPhase phase);
void lab_instruction(const CpuTrace *trace, char *text, size_t capacity);
void lab_explanation(const Playback *lab, char *text, size_t capacity);
#endif
