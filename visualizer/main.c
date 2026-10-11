#include "ui.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool load_file(Playback *lab,const char *path) {
    FILE *file = fopen(path,"rb");
    if (!file) { fprintf(stderr,"Cannot open program: %s\n",path); return false; }
    uint8_t bytes[MEMORY_SIZE+1];
    size_t size = fread(bytes,1,sizeof(bytes),file);
    bool failed = ferror(file) != 0;
    fclose(file);
    if (failed) { fprintf(stderr,"Cannot read program: %s\n",path); return false; }
    lab_load(lab,bytes,size);
    return true;
}
static bool save_frame(const char *path) {
    rlDrawRenderBatchActive();
    Image frame = LoadImageFromScreen();
    bool saved = ExportImage(frame,path);
    UnloadImage(frame);
    return saved;
}
int main(int argc,char **argv) {
    bool smoke = argc >= 2 && strcmp(argv[1],"--smoke-test") == 0;
    const char *capture = smoke && argc >= 3 ? argv[2] : "lab-smoke.png";
    int arg = smoke ? 3 : 1;
    char default_path[1024];
    snprintf(default_path,sizeof(default_path),"%sdemo.bin",GetApplicationDirectory());
    const char *path = argc > arg ? argv[arg] : default_path;
    Playback lab;
    if (!load_file(&lab,path)) return 1;
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | (smoke ? FLAG_WINDOW_HIDDEN:0));
    InitWindow(1280,800,"CPU Lab / Native C");
    if (!IsWindowReady()) return 1;
    SetWindowMinSize(1100,760);
    SetTargetFPS(smoke ? 0 : 60);
    LabScene scene; scene_init(&scene); ui_init();
    RenderTexture2D canvas = LoadRenderTexture((int)ui_viewport().width,(int)ui_viewport().height);
    unsigned frames = 0;
    bool captured = false;
    bool capture_ok = true;
    while (!WindowShouldClose()) {
        if ((smoke && lab.phase == LAB_IDLE && lab.steps < 100) || IsKeyPressed(KEY_SPACE)) lab_step(&lab);
        if (IsKeyPressed(KEY_R)) lab_reset(&lab);
        lab_update(&lab,smoke ? 1.0f/60 : GetFrameTime());
        Rectangle viewport = ui_viewport();
        scene_input(&scene,viewport);
        if (canvas.texture.width != (int)viewport.width || canvas.texture.height != (int)viewport.height) {
            UnloadRenderTexture(canvas);
            canvas = LoadRenderTexture((int)viewport.width,(int)viewport.height);
        }
        BeginTextureMode(canvas);
        ClearBackground((Color){10,15,24,255});
        scene_draw(&scene,&lab);
        EndTextureMode();
        BeginDrawing();
        ClearBackground((Color){10,15,24,255});
        DrawTexturePro(canvas.texture,(Rectangle){0,0,(float)canvas.texture.width,-(float)canvas.texture.height},
            viewport,(Vector2){0,0},0,WHITE);
        scene_labels(&scene,&lab,viewport);
        ui_draw(&lab,GetFileName(path));
        ++frames;
        if (smoke && lab.steps == 3 && lab.phase == LAB_EXECUTE && !captured) {
            capture_ok = save_frame(capture); captured = true;
        }
        bool done = smoke && lab.phase == LAB_IDLE && (lab.cpu.status != CPU_RUNNING || lab.steps >= 100);
        if (done) {
            /* Short/faulting programs still produce the requested capture. */
            if (!captured) {
                capture_ok = save_frame(capture) && capture_ok;
                captured = true;
            }
            char final_capture[1024]; snprintf(final_capture,sizeof(final_capture),"%s.final.png",capture);
            capture_ok = save_frame(final_capture) && capture_ok;
        }
        EndDrawing();
        if (done) break;
        if (smoke && frames > 20000) break;
    }
    if (smoke) {
        printf("Smoke: status=%s PC=%u R0=%u R1=%u steps=%u frames=%u\n",
            cpu_status_message(lab.cpu.status),lab.cpu.pc,lab.cpu.registers[0],lab.cpu.registers[1],lab.steps,frames);
    }
    UnloadRenderTexture(canvas);
    CloseWindow();
    return smoke && (lab.cpu.status != CPU_HALTED || !captured || !capture_ok) ? 1 : 0;
}
