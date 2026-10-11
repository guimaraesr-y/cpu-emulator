#include "ui.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    LabApp app; app_init(&app);
    if (!smoke && argc<=arg) app_example(&app,0);
    else if (!app_open(&app,path)) { fprintf(stderr,"%s\n",app.message); return 1; }
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | (smoke ? FLAG_WINDOW_HIDDEN:0));
    InitWindow(1500,900,"CPU Lab / Native C");
    if (!IsWindowReady()) return 1;
    SetWindowMinSize(1400,850);
    SetTargetFPS(smoke ? 0 : 60);
    LabScene scene; scene_init(&scene); ui_init();
    RenderTexture2D canvas = LoadRenderTexture((int)ui_viewport().width,(int)ui_viewport().height);
    unsigned frames = 0;
    bool captured = false;
    bool capture_ok = true;
    while (!WindowShouldClose()) {
        if (!smoke && IsFileDropped()) {
            FilePathList files=LoadDroppedFiles();
            if (files.count && app_open(&app,files.paths[0])) ui_reset_editor();
            UnloadDroppedFiles(files);
        }
        Playback lab=app.playback;
        if (smoke && lab.phase == LAB_IDLE && lab.steps < 100) lab_step(&lab);
        if (smoke) { app.playback=lab; lab_update(&app.playback,1.0f/60); }
        else {
            if (!ui_editor_active()) {
                if (IsKeyPressed(KEY_SPACE)) app_step(&app);
                if (IsKeyPressed(KEY_P)) app_toggle_play(&app);
                if (IsKeyPressed(KEY_R)) app_reset(&app);
            }
            if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_ENTER)) app_compile(&app);
            app_update(&app,GetFrameTime());
        }
        lab=app.playback;
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
        ui_draw(&app,GetFileName(app.path));
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
        Playback lab=app.playback;
        printf("Smoke: status=%s PC=%u R0=%u R1=%u steps=%u frames=%u\n",
            cpu_status_message(lab.cpu.status),lab.cpu.pc,lab.cpu.registers[0],lab.cpu.registers[1],lab.steps,frames);
    }
    UnloadRenderTexture(canvas);
    CloseWindow();
    return smoke && (app.playback.cpu.status != CPU_HALTED || !captured || !capture_ok) ? 1 : 0;
}
