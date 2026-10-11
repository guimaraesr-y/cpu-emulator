#ifndef LAB_APP_H
#define LAB_APP_H
#include "playback.h"
#include "../assembler/asm.h"
#define LAB_SOURCE_CAPACITY 32768
#define LAB_STEP_LIMIT 100000u
typedef struct {
    Playback playback;
    AsmResult assembly;
    char source[LAB_SOURCE_CAPACITY], compiled[LAB_SOURCE_CAPACITY];
    char path[1024], message[512];
    bool has_source, playing, manual, limited;
    float speed;
    unsigned memory_page;
} LabApp;
void app_init(LabApp *app);
bool app_compile(LabApp *app);
bool app_open(LabApp *app, const char *path);
bool app_save(LabApp *app, const char *path);
void app_reset(LabApp *app);
void app_step(LabApp *app);
void app_toggle_play(LabApp *app);
void app_update(LabApp *app, float seconds);
bool app_dirty(const LabApp *app);
size_t app_current_line(const LabApp *app);
void app_example(LabApp *app, unsigned index);
#endif
