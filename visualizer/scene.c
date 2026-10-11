#include "scene.h"
#include "raymath.h"
#include "../opcodes.h"
#include <math.h>

static const Color teal = {62,230,197,255}, violet = {156,132,255,255};
static const Vector3 ram = {-4,0.7f,0}, pc = {-3,0.7f,3}, control = {0,0.7f,0};
static const Vector3 alu = {3.8f,0.7f,0}, regs = {0,0.7f,-3};

static void position_camera(LabScene *s) {
    Vector3 offset = {cosf(s->yaw)*cosf(s->pitch)*s->distance,
        sinf(s->pitch)*s->distance, sinf(s->yaw)*cosf(s->pitch)*s->distance};
    s->camera.position = Vector3Add(s->camera.target, offset);
}
void scene_init(LabScene *s) {
    s->camera = (Camera3D){{0,0,0},{0,0,0},{0,1,0},42,CAMERA_PERSPECTIVE};
    s->yaw = 1.08f; s->pitch = 0.78f; s->distance = 17;
    position_camera(s);
}
void scene_input(LabScene *s, Rectangle viewport) {
    if (!CheckCollisionPointRec(GetMousePosition(), viewport)) return;
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        s->yaw -= delta.x * 0.006f;
        s->pitch = Clamp(s->pitch + delta.y * 0.006f, 0.2f, 1.4f);
    }
    s->distance = Clamp(s->distance - GetMouseWheelMove(), 7, 28);
    if (IsKeyPressed(KEY_HOME)) scene_init(s);
    if (IsKeyPressed(KEY_ONE)) s->camera.target = ram;
    if (IsKeyPressed(KEY_TWO)) s->camera.target = control;
    if (IsKeyPressed(KEY_THREE)) s->camera.target = alu;
    if (IsKeyPressed(KEY_FOUR)) s->camera.target = regs;
    position_camera(s);
}
static void chip(Vector3 p, Vector3 size, bool active, Color accent) {
    DrawCubeV((Vector3){p.x,0.1f,p.z},(Vector3){size.x+0.2f,0.18f,size.z+0.2f},(Color){32,52,65,255});
    DrawCubeV(p,size,active ? (Color){30,76,83,255} : (Color){24,35,52,255});
    DrawCubeWiresV(p,size,active ? accent : (Color){65,87,111,255});
    for (int i=0;i<5;i++) {
        float x = p.x - size.x/2 + (i+0.5f)*size.x/5;
        DrawCube((Vector3){x,0.25f,p.z+size.z/2+0.12f},0.1f,0.15f,0.24f,(Color){130,141,153,255});
        DrawCube((Vector3){x,0.25f,p.z-size.z/2-0.12f},0.1f,0.15f,0.24f,(Color){130,141,153,255});
    }
}
static void bus(Vector3 a, Vector3 b, bool active, float progress) {
    a.y = b.y = 0.25f;
    DrawCylinderEx(a,b,0.027f,0.027f,8,(Color){42,82,83,255});
    if (active) {
        Vector3 pulse = Vector3Lerp(a,b,Clamp(progress,0,1)); pulse.y += 0.04f;
        DrawSphere(pulse,0.12f,teal);
        DrawSphereWires(pulse,0.2f,8,8,Fade(teal,0.5f));
    }
}
void scene_draw(const LabScene *s, const Playback *lab) {
    const CpuTrace *t = &lab->trace;
    bool good = t->fetched && (t->after.status == CPU_RUNNING || t->after.status == CPU_HALTED);
    bool arithmetic = good && (t->bytes[0] == OPCODE_ADD || t->bytes[0] == OPCODE_SUB || t->bytes[0] == OPCODE_XOR);
    float progress = lab->elapsed/0.55f;
    BeginMode3D(s->camera);
    DrawPlane((Vector3){0,-0.3f,0},(Vector2){200,200},(Color){10,15,24,255});
    DrawCube((Vector3){0,-0.15f,0},12,0.2f,9,(Color){17,30,42,255});
    DrawCubeWires((Vector3){0,-0.15f,0},12,0.2f,9,(Color){41,65,83,255});
    for (int i=-5;i<=5;i++) DrawLine3D((Vector3){(float)i,-0.04f,-4},(Vector3){(float)i,-0.04f,4},(Color){23,40,53,255});
    bus(pc,ram,lab->phase == LAB_FETCH && t->fetched,progress);
    bus(ram,control,lab->phase == LAB_FETCH && t->fetched,progress);
    bus(control,regs,lab->phase == LAB_EXECUTE && good && t->bytes[0] == OPCODE_MOV,progress);
    bus(regs,alu,lab->phase == LAB_EXECUTE && arithmetic,progress);
    bus(alu,regs,lab->phase == LAB_WRITEBACK && arithmetic,progress);
    bus(control,pc,lab->phase == LAB_EXECUTE && good && t->bytes[0] == OPCODE_JMP,progress);
    chip(ram,(Vector3){2.4f,0.65f,3.1f},lab->phase == LAB_FETCH && t->fetched,teal);
    chip(pc,(Vector3){1.8f,0.65f,1},lab->phase == LAB_WRITEBACK,teal);
    chip(control,(Vector3){2.2f,0.9f,1.8f},lab->phase == LAB_DECODE,violet);
    chip(alu,(Vector3){2.0f,0.85f,2},lab->phase == LAB_EXECUTE && arithmetic,violet);
    for (int i=0;i<NUM_REGISTERS;i++) {
        Vector3 r = {-2.1f+(i%4)*1.4f,0.65f,-2.6f-(i/4)*1.05f};
        bool changed = (lab->phase == LAB_WRITEBACK || lab->phase == LAB_IDLE) && (t->changed & (1u<<i));
        chip(r,(Vector3){1.1f,0.55f,0.65f},changed,teal);
    }
    EndMode3D();
}
static void label(const LabScene *s, Vector3 p, const char *text, Color color, Rectangle viewport) {
    Vector2 v = GetWorldToScreenEx(p,s->camera,(int)viewport.width,(int)viewport.height);
    v.x += viewport.x; v.y += viewport.y;
    int width = MeasureText(text,15);
    if (!CheckCollisionPointRec(v,viewport) || v.x-width/2 < viewport.x || v.x+width/2 > viewport.x+viewport.width) return;
    DrawRectangle((int)v.x-width/2-5,(int)v.y-3,width+10,22,Fade((Color){10,17,28,255},0.92f));
    DrawText(text,(int)v.x-width/2,(int)v.y,15,color);
}
void scene_labels(const LabScene *s, const Playback *lab, Rectangle viewport) {
    const Cpu *c = lab_visible(lab);
    label(s,(Vector3){ram.x,1.3f,ram.z},"RAM / 1024 B",teal,viewport);
    label(s,(Vector3){pc.x,1.3f,pc.z},TextFormat("PC / %u",c->pc),teal,viewport);
    label(s,(Vector3){0,1.55f,0},"CONTROL UNIT",violet,viewport);
    label(s,(Vector3){alu.x,1.4f,0},"ALU / 8 BIT",violet,viewport);
    for (int i=0;i<NUM_REGISTERS;i++)
        label(s,(Vector3){-2.1f+(i%4)*1.4f,1.05f,-2.6f-(i/4)*1.05f},TextFormat("R%d / %u",i,c->registers[i]),WHITE,viewport);
    const CpuTrace *t = &lab->trace;
    bool good = t->fetched && (t->after.status == CPU_RUNNING || t->after.status == CPU_HALTED);
    if (lab->phase == LAB_EXECUTE && good) {
        if (t->bytes[0] == OPCODE_ADD || t->bytes[0] == OPCODE_SUB || t->bytes[0] == OPCODE_XOR)
            label(s,(Vector3){2,0.7f,-1.6f},TextFormat("%u, %u -> %u",t->before.registers[t->bytes[1]],
                t->before.registers[t->bytes[2]],t->after.registers[t->bytes[1]]),teal,viewport);
        else if (t->bytes[0] == OPCODE_MOV)
            label(s,(Vector3){0,0.7f,-1.6f},TextFormat("immediate %u -> R%u",t->bytes[2],t->bytes[1]),teal,viewport);
    }
}
