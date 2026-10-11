#include "ui.h"
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
Rectangle ui_viewport(void) { return (Rectangle){0,96,(float)GetScreenWidth()-350,(float)GetScreenHeight()-180}; }
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
void ui_draw(Playback *lab, const char *program_name) {
    int w = GetScreenWidth(), h = GetScreenHeight(), x = w-330;
    DrawRectangle(0,0,w,88,(Color){13,21,34,255});
    DrawText("CPU / LAB",28,20,30,ink);
    DrawText("NATIVE C   /   ISA v1   /   INSTRUCTION EXPLORER",28,58,14,muted);
    const char *phase = lab_phase_name(lab->phase);
    if (lab->phase == LAB_IDLE && lab->cpu.status != CPU_RUNNING)
        phase = lab->cpu.status == CPU_HALTED ? "HALTED" : "FAULT";
    DrawText(phase,w-190,30,22,teal);
    DrawRectangle(w-350,96,350,h-180,(Color){13,21,34,255});
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
    DrawText("RAM / instruction window",28,112,14,teal);
    unsigned address = c->pc;
    if (address >= c->program_size && c->program_size >= 3) address = (unsigned)c->program_size-3;
    unsigned start = address >= 6 ? address-6 : 0;
    for (unsigned row=0;row<5;row++) {
        unsigned at = start+row*3;
        if (at+2>=c->program_size) break;
        DrawText(TextFormat("%03u    %02X %02X %02X",at,c->memory[at],c->memory[at+1],c->memory[at+2]),28,140+(int)row*23,17,at==c->pc ? teal:muted);
    }
    DrawRectangle(0,h-80,w,80,(Color){13,21,34,255});
    if (lab->phase != LAB_IDLE || lab->cpu.status != CPU_RUNNING) GuiDisable();
    bool step = GuiButton((Rectangle){28,(float)h-62,145,40},"STEP / SPACE");
    GuiEnable();
    if (step) lab_step(lab);
    if (GuiButton((Rectangle){185,(float)h-62,115,40},"RESET / R")) lab_reset(lab);
    DrawText("RMB: orbit   Wheel: zoom   1-4: focus   Home: overview",325,h-56,14,muted);
    wrap("Functional model. Phases are didactic; one atomic CPU step.",325,h-34,w-350,13,muted);
    Color status = c->status == CPU_RUNNING ? muted : c->status == CPU_HALTED ? teal : (Color){255,138,122,255};
    wrap(cpu_status_message(c->status),x,h-155,310,15,status);
}
