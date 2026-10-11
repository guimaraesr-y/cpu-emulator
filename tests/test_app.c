#include "../visualizer/app.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    LabApp app; app_init(&app); app_example(&app,0);
    LabApp coarse=app, fine=app;
    app_toggle_play(&coarse); app_toggle_play(&fine);
    app_update(&coarse,10);
    for (int i=0;i<100;i++) app_update(&fine,0.1f);
    assert(coarse.playback.steps==fine.playback.steps);
    assert(coarse.playback.phase==fine.playback.phase);
    assert(!memcmp(&coarse.playback.cpu,&fine.playback.cpu,sizeof(Cpu)));
    assert(app.playback.cpu.pc==0 && app_current_line(&app)==2);
    app_toggle_play(&app); app_update(&app,0.2f); assert(app.playback.steps==1);
    app_toggle_play(&app); float elapsed=app.playback.elapsed;
    app_update(&app,100); assert(app.playback.elapsed==elapsed && app.playback.steps==1);
    app_step(&app); app_update(&app,3); assert(app.playback.phase==LAB_IDLE && app.playback.steps==1);
    app_toggle_play(&app);
    for (int i=0;i<20;i++) app_update(&app,3);
    assert(app.playback.cpu.status==CPU_HALTED && app.playback.cpu.registers[0]==30 && app.playback.cpu.registers[1]==246);
    assert(!app.playing); app_reset(&app); assert(app.playback.steps==0);
    strcpy(app.source,"mov R9, 1\nhlt\n"); assert(!app_compile(&app));
    assert(app_dirty(&app) && strstr(app.message,"Line 1"));
    app_toggle_play(&app); app_step(&app); app_update(&app,10); assert(app.playback.steps==0);
    strcpy(app.source,"; comment\njmp end\nmov R0, 99\nend: hlt\n");
    assert(app_compile(&app)); assert(app.assembly.lines[0]==2 && app.assembly.lines[2]==4);
    app_step(&app); app_update(&app,3); assert(app_current_line(&app)==4);
    app_example(&app,1); app.speed=4; app_step(&app); app_update(&app,0.6f);
    assert(app.playback.phase==LAB_IDLE && app.playback.steps==1);
    app_example(&app,2); app.playback.steps=LAB_STEP_LIMIT; app_toggle_play(&app); app_update(&app,1);
    assert(app.limited && !app.playing); app_reset(&app); assert(!app.limited);
    FILE *file=fopen("app-test.asm","wb"); assert(file);
    const char *source="; CRLF\r\nmov R0, 42\r\nhlt\r\n";
    assert(fwrite(source,1,strlen(source),file)==strlen(source)); assert(!fclose(file));
    assert(app_open(&app,"app-test.asm")); assert(!strchr(app.source,'\r'));
    assert(app.assembly.lines[0]==2 && app.playback.cpu.status==CPU_RUNNING);
    app_step(&app); app_update(&app,3); assert(app.playback.cpu.registers[0]==42);
    assert(!app_open(&app,"missing-file.asm") && app.playback.cpu.registers[0]==42);
    assert(!app_save(&app,"app-test.bin")); assert(app_save(&app,"app-test.asm"));
    assert(!remove("app-test.asm"));
    puts("App checks passed: pause/resume, step, dirty source, source map, speed, limit.");
    return 0;
}
