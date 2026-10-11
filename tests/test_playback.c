#include "../visualizer/playback.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void finish(Playback *p) { lab_update(p, 3.0f); assert(p->phase == LAB_IDLE); }
int main(void) {
    Playback p;
    uint8_t code[] = {1,0,10, 1,1,20, 2,0,1, 3,1,0, 5,0,1, 4,21,0, 1,2,99, 255,0,0};
    assert(lab_load(&p, code, sizeof(code)) == CPU_RUNNING);
    assert(lab_step(&p));
    assert(p.cpu.pc == 3 && p.cpu.registers[0] == 10);
    assert(lab_visible(&p)->pc == 0 && lab_visible(&p)->registers[0] == 0);
    assert(!lab_step(&p) && p.steps == 1);
    lab_update(&p, 0.6f); assert(p.phase == LAB_DECODE);
    lab_update(&p, 0.6f); assert(p.phase == LAB_EXECUTE);
    lab_update(&p, 0.6f); assert(p.phase == LAB_WRITEBACK);
    assert(lab_visible(&p)->registers[0] == 10 && p.trace.changed == 1);
    finish(&p);
    assert(lab_step(&p)); finish(&p);
    assert(lab_step(&p)); assert(p.trace.before.registers[0] == 10 && p.trace.after.registers[0] == 30); finish(&p);
    assert(lab_step(&p)); assert(p.cpu.registers[1] == 246); finish(&p);
    assert(lab_step(&p)); assert(p.cpu.registers[0] == (30 ^ 246)); finish(&p);
    assert(lab_step(&p)); assert(p.trace.before.pc == 15 && p.trace.after.pc == 21); finish(&p);
    assert(lab_step(&p)); finish(&p);
    assert(p.cpu.status == CPU_HALTED && p.cpu.pc == 24 && p.cpu.registers[2] == 0);
    assert(!lab_step(&p));
    lab_reset(&p); assert(p.steps == 0 && p.cpu.pc == 0 && p.cpu.registers[0] == 0);
    assert(lab_step(&p)); lab_reset(&p); assert(p.phase == LAB_IDLE && p.cpu.pc == 0);
    uint8_t same[] = {1,0,0,255,0,0};
    lab_load(&p, same, sizeof(same)); lab_step(&p); assert(p.trace.changed == 0);
    uint8_t invalid[][3] = {{1,8,0},{2,0,8},{4,1,0},{255,0,1},{254,0,0}};
    for (unsigned i=0; i<sizeof(invalid)/sizeof(invalid[0]); ++i) {
        char explanation[256];
        lab_load(&p, invalid[i], 3); lab_step(&p);
        lab_explanation(&p, explanation, sizeof(explanation));
        assert(strstr(explanation,"Fault") && p.cpu.pc == 0 && p.trace.changed == 0);
        finish(&p); assert(!lab_step(&p));
    }
    uint8_t missing_halt[] = {1,0,7};
    lab_load(&p, missing_halt, 3); lab_step(&p); finish(&p);
    assert(lab_step(&p)); assert(!p.trace.fetched && p.cpu.status == CPU_BAD_PC); finish(&p);
    uint8_t far[279] = {4,17,1};
    far[273]=1; far[274]=0; far[275]=42; far[276]=255;
    lab_load(&p, far, sizeof(far)); lab_step(&p); assert(p.cpu.pc == 273); finish(&p);
    lab_step(&p); assert(p.cpu.registers[0] == 42);
    assert(lab_load(&p, NULL, 0) == CPU_BAD_PROGRAM && !lab_step(&p));
    puts("Playback checks passed: snapshots, timing, reset, all opcodes, faults, 16-bit jump.");
    return 0;
}
