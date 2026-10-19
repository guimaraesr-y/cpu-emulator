#include "app.h"
#include <stdio.h>
#include <string.h>

void app_init(LabApp *app) { memset(app,0,sizeof(*app)); app->speed=1; }
bool app_dirty(const LabApp *app) { return app->has_source && strcmp(app->source,app->compiled)!=0; }
bool app_compile(LabApp *app) {
    AsmResult assembled;
    app->playing=false;
    if (!assemble_text(app->source,&assembled)) {
        snprintf(app->message,sizeof(app->message),"Line %zu: %s",assembled.error_line,assembled.error);
        return false;
    }
    app->assembly=assembled;
    memcpy(app->compiled,app->source,sizeof(app->compiled));
    app->has_source=true;
    lab_load(&app->playback,assembled.bytes,assembled.size);
    app->manual=false; app->limited=false;
    snprintf(app->message,sizeof(app->message),"Assembled %zu instructions. Ready.",assembled.size/3);
    return true;
}
bool app_open(LabApp *app,const char *path) {
    FILE *file=fopen(path,"rb");
    if (!file) { snprintf(app->message,sizeof(app->message),"Cannot open file: %s",path); return false; }
    const char *ext=strrchr(path,'.');
    bool binary=ext && (!strcmp(ext,".bin") || !strcmp(ext,".BIN"));
    char source[LAB_SOURCE_CAPACITY]; uint8_t bytes[MEMORY_SIZE+1];
    size_t size=fread(binary ? (void*)bytes : (void*)source,1,binary ? sizeof(bytes) : sizeof(source)-1,file);
    bool failed=ferror(file)!=0;
    bool oversized=!binary && size==sizeof(source)-1 && fgetc(file)!=EOF;
    if (fclose(file)!=0) failed=true;
    if (failed || oversized) { snprintf(app->message,sizeof(app->message),"Cannot read file or source exceeds editor capacity."); return false; }
    app->playing=false; app->manual=false; app->limited=false;
    if (binary) {
        lab_load(&app->playback,bytes,size);
        app->has_source=false; app->source[0]=app->compiled[0]=0;
        memset(&app->assembly,0,sizeof(app->assembly));
        snprintf(app->message,sizeof(app->message),"Loaded bytecode: %s",cpu_status_message(app->playback.cpu.status));
    } else {
        /* Normalize Windows text files for the multiline editor/source map. */
        size_t out=0;
        for (size_t i=0;i<size;i++) if (source[i]!='\r') source[out++]=source[i];
        source[out]=0;
        memcpy(app->source,source,out+1); app->has_source=true;
        if (!app_compile(app)) {
            /* Do not execute a previous file after a failed source load. */
            lab_load(&app->playback,NULL,0); app->compiled[0]=0;
            memset(&app->assembly,0,sizeof(app->assembly));
        }
    }
    /* The UI may pass app->path itself. */
    if (path!=app->path) snprintf(app->path,sizeof(app->path),"%s",path);
    app->memory_page=0;
    return true;
}
bool app_save(LabApp *app,const char *path) {
    if (!app->has_source) { snprintf(app->message,sizeof(app->message),"No Assembly source to save."); return false; }
    const char *ext=strrchr(path,'.');
    if (!ext || (strcmp(ext,".asm") && strcmp(ext,".ASM"))) {
        snprintf(app->message,sizeof(app->message),"Save Assembly to a .asm file."); return false;
    }
    FILE *file=fopen(path,"wb");
    if (!file) { snprintf(app->message,sizeof(app->message),"Cannot save source."); return false; }
    size_t length=strlen(app->source);
    bool ok=fwrite(app->source,1,length,file)==length;
    if (fclose(file)) ok=false;
    snprintf(app->message,sizeof(app->message),"%s",ok ? "Source saved. Assemble to load changes." : "Failed to save source.");
    return ok;
}
void app_reset(LabApp *app) {
    lab_reset(&app->playback); app->playing=false; app->manual=false; app->limited=false;
    snprintf(app->message,sizeof(app->message),"Reset loaded program.");
}
void app_step(LabApp *app) {
    app->playing=false;
    if (app_dirty(app) || app->limited) return;
    if (app->playback.phase==LAB_IDLE) app->manual=lab_step(&app->playback);
    else app->manual=true; /* Finish a paused instruction; do not execute twice. */
}
void app_toggle_play(LabApp *app) {
    if (app_dirty(app) || app->limited) return;
    if (app->playback.phase==LAB_IDLE && app->playback.cpu.status!=CPU_RUNNING) return;
    app->playing=!app->playing; app->manual=false;
}
void app_update(LabApp *app,float seconds) {
    if (app_dirty(app)) { app->playing=false; app->manual=false; return; }
    float time=seconds*app->speed;
    while (time>0 && (app->playing || app->manual)) {
        if (app->playback.phase==LAB_IDLE) {
            if (!app->playing) { app->manual=false; break; }
            if (app->playback.steps>=LAB_STEP_LIMIT) {
                app->playing=false; app->limited=true;
                snprintf(app->message,sizeof(app->message),"Instruction limit reached. Reset to continue."); break;
            }
            if (!lab_step(&app->playback)) { app->playing=false; break; }
        }
        float remaining=(float)(LAB_WRITEBACK-app->playback.phase+1)*0.55f-app->playback.elapsed;
        if (time<remaining) { lab_update(&app->playback,time); break; }
        /* Carry unused frame time into the next instruction, independent of FPS. */
        time-=remaining; app->playback.phase=LAB_IDLE; app->playback.elapsed=0;
        app->manual=false;
        if (app->playback.cpu.status!=CPU_RUNNING) app->playing=false;
    }
}
size_t app_current_line(const LabApp *app) {
    if (!app->has_source || app_dirty(app)) return 0;
    size_t pc=lab_visible(&app->playback)->pc;
    if (app->playback.cpu.status==CPU_HALTED && app->playback.trace.fetched)
        pc=app->playback.trace.before.pc;
    return pc<app->assembly.size && pc%3==0 ? app->assembly.lines[pc/3] : 0;
}
void app_example(LabApp *app,unsigned index) {
    const char *examples[]={
        "; Addition and unsigned wraparound\nmov R0, 10\nmov R1, 20\nadd R0, R1\nsub R1, R0\njmp end\nmov R2, 99\nend: hlt\n",
        "; XOR example\nmov R0, 3\nmov R1, 5\nxor R0, R1\nhlt\n",
        "; Loop: pause or reset at any time\nloop: add R0, R1\njmp loop\n"};
    snprintf(app->source,sizeof(app->source),"%s",examples[index%3]);
    snprintf(app->path,sizeof(app->path),"example-%u.asm",index%3+1);
    app->has_source=true; app_compile(app);
}
