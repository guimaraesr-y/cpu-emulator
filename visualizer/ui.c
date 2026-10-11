#include "ui.h"
#include "editor.h"
#define RAYGUI_IMPLEMENTATION
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#include "raygui.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include <stdio.h>
#include <string.h>

static const Color ink = {220,230,244,255}, muted = {133,153,177,255}, teal = {62,230,197,255};
static LabEditor editor;
static bool path_edit;
static int tab, example;
void ui_reset_editor(void) { memset(&editor,0,sizeof(editor)); path_edit=false; }
bool ui_editor_active(void) { return editor.focused || path_edit; }
Rectangle ui_viewport(void) { return (Rectangle){360,96,(float)GetScreenWidth()-710,(float)GetScreenHeight()-210}; }
void ui_init(void) {
    GuiSetStyle(DEFAULT,TEXT_SIZE,16);
    GuiSetStyle(DEFAULT,BASE_COLOR_NORMAL,0x18353fff);
    GuiSetStyle(DEFAULT,TEXT_COLOR_NORMAL,0xdce6f4ff);
    GuiSetStyle(DEFAULT,BORDER_COLOR_NORMAL,0x386979ff);
    GuiSetStyle(DEFAULT,BASE_COLOR_FOCUSED,0x28575bff);
    GuiSetStyle(DEFAULT,TEXT_COLOR_FOCUSED,0x3ee6c5ff);
    GuiSetStyle(DEFAULT,BASE_COLOR_DISABLED,0x142531ff);
    GuiSetStyle(DEFAULT,TEXT_COLOR_DISABLED,0x728898ff);
    GuiSetStyle(DEFAULT,BORDER_COLOR_DISABLED,0x29434fff);
}
static int wrap(const char *text,int x,int y,int width,int size,Color color) {
    char line[256] = ""; size_t length = 0;
    for (const char *p=text; *p; ++p) {
        line[length++] = *p; line[length] = 0;
        if (length >= sizeof(line)-2 || MeasureText(line,size) > width || *p=='\n') {
            char *space = strrchr(line,' ');
            if (space && *p != '\n') {
                size_t consumed = (size_t)(space-line)+1;
                *space = 0; DrawText(line,x,y,size,color);
                memmove(line,line+consumed,length-consumed+1); length -= consumed;
            } else {
                DrawText(line,x,y,size,color); length=0; line[0]=0;
            }
            y+=size+6;
        }
    }
    if (length) { DrawText(line,x,y,size,color); y+=size+6; }
    return y;
}
static void workspace(LabApp *app) {
    int h=GetScreenHeight();
    DrawRectangle(0,96,350,h-210,(Color){13,21,34,255});
    GuiToggleGroup((Rectangle){16,106,140,28},"ASSEMBLY;RAM",&tab);
    if (!tab) {
        if (GuiComboBox((Rectangle){16,148,195,28},"Addition;XOR;Loop",&example)) {
            app_example(app,(unsigned)example); memset(&editor,0,sizeof(editor));
        }
        if (GuiButton((Rectangle){222,148,110,28},"ASSEMBLE")) { app_compile(app); editor.focused=false; }
        editor_draw(&editor,app,(Rectangle){16,190,316,(float)h-415});
        DrawText(app_dirty(app) ? "Edited / assemble before running" : "Current line follows loaded program",16,h-213,14,(Color){62,230,197,255});
        if (!app->has_source) DrawText("Binary has no source map. Pick an example.",16,183,12,(Color){255,138,122,255});
    } else {
        editor.focused=false;
        if (GuiButton((Rectangle){16,148,65,28},"PREV") && app->memory_page) --app->memory_page;
        if (GuiButton((Rectangle){262,148,65,28},"NEXT") && app->memory_page<7) ++app->memory_page;
        GuiLabel((Rectangle){93,148,162,28},TextFormat("RAM page %u / 8",app->memory_page+1));
        const Cpu *cpu=lab_visible(&app->playback);
        unsigned base=app->memory_page*128;
        for (unsigned row=0;row<16;row++) {
            unsigned addr=base+row*8; int y=196+(int)row*24;
            DrawText(TextFormat("%03X",addr),16,y,14,(Color){102,127,151,255});
            for (unsigned col=0;col<8;col++) {
                unsigned at=addr+col;
                bool fetched=app->playback.trace.fetched && app->playback.phase!=LAB_IDLE &&
                    at>=app->playback.trace.before.pc && at<app->playback.trace.before.pc+3u;
                Color color=fetched ? (Color){62,230,197,255} : at<cpu->program_size ?
                    (Color){220,230,244,255} : (Color){74,95,119,255};
                DrawText(TextFormat("%02X",cpu->memory[at]),60+(int)col*34,y,14,color);
            }
        }
        DrawText("Addresses and bytes in hexadecimal",16,600,14,(Color){102,127,151,255});
    }
    if (GuiTextBox((Rectangle){16,(float)h-182,316,28},app->path,sizeof(app->path),path_edit)) path_edit=!path_edit;
    if (GuiButton((Rectangle){16,(float)h-146,145,28},"OPEN .asm/.bin")) {
        app_open(app,app->path); memset(&editor,0,sizeof(editor)); path_edit=false;
    }
    if (GuiButton((Rectangle){173,(float)h-146,159,28},"SAVE .asm")) app_save(app,app->path);
}
void ui_draw(LabApp *app, const char *program_name) {
    Playback *lab=&app->playback;
    int w = GetScreenWidth(), h = GetScreenHeight(), x = w-330;
    DrawRectangle(0,0,w,88,(Color){13,21,34,255});
    DrawText("CPU / LAB",28,20,30,ink);
    DrawText("NATIVE C   /   ISA v1   /   INSTRUCTION EXPLORER",28,58,14,muted);
    const char *phase = lab_phase_name(lab->phase);
    if (lab->phase == LAB_IDLE && lab->cpu.status != CPU_RUNNING)
        phase = lab->cpu.status == CPU_HALTED ? "HALTED" : "FAULT";
    DrawText(phase,w-190,30,22,teal);
    DrawRectangle(w-350,96,350,h-210,(Color){13,21,34,255});
    DrawText("PROGRAM",x,116,14,teal);
    wrap(program_name,x,142,310,18,ink);
    const Cpu *c = lab_visible(lab);
    char instruction[96], explanation[256];
    lab_instruction(&lab->trace,instruction,sizeof(instruction));
    if (!lab->steps) snprintf(instruction,sizeof(instruction),"Ready for first instruction");
    DrawText(instruction,x,180,20,ink);
    DrawText(TextFormat("PC %03u   |   %u instruction(s)",c->pc,lab->steps),x,215,16,muted);
    if (lab->steps && lab->trace.fetched)
        DrawText(TextFormat("BYTES   %02X %02X %02X",lab->trace.bytes[0],lab->trace.bytes[1],lab->trace.bytes[2]),x,245,18,teal);
    lab_explanation(lab,explanation,sizeof(explanation));
    wrap(explanation,x,280,310,16,ink);
    DrawText("REGISTERS  /  unsigned 8-bit",x,350,14,muted);
    for (unsigned i=0;i<NUM_REGISTERS;i++) {
        char bits[9]; for (unsigned j=0;j<8;j++) bits[j]=(c->registers[i] & (1u<<(7-j))) ? '1':'0'; bits[8]=0;
        bool changed = (lab->phase == LAB_WRITEBACK || lab->phase == LAB_IDLE) && (lab->trace.changed & (1u<<i));
        DrawText(TextFormat("R%u   %3u   0x%02X   %s",i,c->registers[i],c->registers[i],bits),x,380+(int)i*25,16,changed ? teal:ink);
    }
    DrawRectangle(0,h-110,w,110,(Color){13,21,34,255});
    if (app_dirty(app) || app->limited || (lab->phase==LAB_IDLE && lab->cpu.status!=CPU_RUNNING)) GuiDisable();
    if (GuiButton((Rectangle){16,(float)h-98,120,36},app->playing ? "PAUSE / P":"PLAY / P")) app_toggle_play(app);
    if (GuiButton((Rectangle){148,(float)h-98,120,36},"STEP / SPACE")) app_step(app);
    GuiEnable();
    if (GuiButton((Rectangle){280,(float)h-98,110,36},"RESET / R")) app_reset(app);
    GuiSlider((Rectangle){465,(float)h-91,200,24},"SPEED",TextFormat("%.2fx",app->speed),&app->speed,0.25f,8);
    wrap(app->message,16,h-48,w-32,14,app_dirty(app) ? (Color){255,183,117,255}:muted);
    DrawText("RMB: orbit / Wheel: zoom / 1-4: focus / Home: overview",720,h-89,14,muted);
    DrawText("Didactic phases / atomic CPU step",720,h-66,13,muted);
    Color status = c->status == CPU_RUNNING ? muted : c->status == CPU_HALTED ? teal : (Color){255,138,122,255};
    wrap(cpu_status_message(c->status),x,h-180,310,15,status);
    workspace(app);
}
