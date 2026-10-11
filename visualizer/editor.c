#include "editor.h"
#include "raymath.h"
#include <string.h>
#include <stdio.h>

static size_t line_start(const char *source,size_t cursor) {
    while (cursor && source[cursor-1]!='\n') --cursor;
    return cursor;
}
static void insert(LabEditor *e,LabApp *app,const char *text) {
    if (!text) return;
    size_t length=strlen(app->source), count=strlen(text);
    if (count>=sizeof(app->source)-(e->select_all ? 0:length)) {
        snprintf(app->message,sizeof(app->message),"Editor capacity exceeded."); return;
    }
    if (e->select_all) { app->source[0]=0; length=0; e->cursor=0; e->select_all=false; }
    char normalized[LAB_SOURCE_CAPACITY]; size_t out=0;
    for (size_t i=0;i<count;i++) if (text[i]!='\r') normalized[out++]=text[i];
    text=normalized; count=out;
    memmove(app->source+e->cursor+count,app->source+e->cursor,length-e->cursor+1);
    memcpy(app->source+e->cursor,text,count); e->cursor+=count;
}
void editor_draw(LabEditor *e,LabApp *app,Rectangle bounds) {
    size_t length=strlen(app->source);
    if (e->cursor>length) e->cursor=length;
    Vector2 mouse=GetMousePosition();
    bool inside=CheckCollisionPointRec(mouse,bounds);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        e->focused=inside;
        if (inside) {
            unsigned target=e->top+(unsigned)((mouse.y-bounds.y-8)/22);
            size_t at=0; for (unsigned row=0;row<target && app->source[at];row++) {
                while (app->source[at] && app->source[at]!='\n') ++at;
                if (app->source[at]) ++at;
            }
            char text[512]; size_t col=0;
            while (app->source[at+col] && app->source[at+col]!='\n' && col<sizeof(text)-1) {
                text[col]=app->source[at+col]; text[col+1]=0;
                if (MeasureText(text,16)>mouse.x-bounds.x-45) break;
                ++col;
            }
            e->cursor=at+col; e->select_all=false;
        }
    }
    unsigned lines=1; for (size_t i=0;i<length;i++) if (app->source[i]=='\n') ++lines;
    float wheel=inside ? GetMouseWheelMove():0;
    if (inside) {
        int top=(int)e->top-(int)(wheel*3);
        e->top=(unsigned)Clamp((float)top,0,(float)(lines-1));
    }
    if (e->focused && app->has_source) {
        bool ctrl=IsKeyDown(KEY_LEFT_CONTROL)||IsKeyDown(KEY_RIGHT_CONTROL);
        if (ctrl && IsKeyPressed(KEY_A)) e->select_all=true;
        if (ctrl && IsKeyPressed(KEY_C)) SetClipboardText(app->source);
        if (ctrl && IsKeyPressed(KEY_V)) insert(e,app,GetClipboardText());
        if (ctrl && IsKeyPressed(KEY_X) && e->select_all) {
            SetClipboardText(app->source); app->source[0]=0; e->cursor=0; e->select_all=false;
        }
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) { if (e->cursor) --e->cursor; e->select_all=false; }
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) { if (e->cursor<strlen(app->source)) ++e->cursor; e->select_all=false; }
        if (IsKeyPressed(KEY_HOME)) { e->cursor=line_start(app->source,e->cursor); e->select_all=false; }
        if (IsKeyPressed(KEY_END)) { while (app->source[e->cursor] && app->source[e->cursor]!='\n') ++e->cursor; e->select_all=false; }
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
            size_t start=line_start(app->source,e->cursor), column=e->cursor-start, next=start;
            if (IsKeyPressed(KEY_UP) && start) next=line_start(app->source,start-1);
            if (IsKeyPressed(KEY_DOWN)) { while (app->source[next] && app->source[next]!='\n') ++next; if (app->source[next]) ++next; }
            e->cursor=next; while (column-- && app->source[e->cursor] && app->source[e->cursor]!='\n') ++e->cursor;
            e->select_all=false;
        }
        bool back=IsKeyPressed(KEY_BACKSPACE)||IsKeyPressedRepeat(KEY_BACKSPACE);
        bool del=IsKeyPressed(KEY_DELETE)||IsKeyPressedRepeat(KEY_DELETE);
        if ((back||del) && e->select_all) { app->source[0]=0; e->cursor=0; e->select_all=false; }
        else if (back && e->cursor) {
            memmove(app->source+e->cursor-1,app->source+e->cursor,strlen(app->source)-e->cursor+1); --e->cursor;
        } else if (del && app->source[e->cursor])
            memmove(app->source+e->cursor,app->source+e->cursor+1,strlen(app->source)-e->cursor);
        if (!ctrl && IsKeyPressed(KEY_ENTER)) insert(e,app,"\n");
        if (IsKeyPressed(KEY_TAB)) insert(e,app,"    ");
        int ch; while ((ch=GetCharPressed())) if (!ctrl && ch>=32 && ch<127) { char text[2]={(char)ch,0}; insert(e,app,text); }
        unsigned cursor_line=0; for (size_t i=0;i<e->cursor;i++) if (app->source[i]=='\n') ++cursor_line;
        unsigned visible=(unsigned)(bounds.height/22)-1;
        if (!wheel) {
            if (cursor_line<e->top) e->top=cursor_line;
            if (cursor_line>=e->top+visible) e->top=cursor_line-visible+1;
        }
    }
    DrawRectangleRec(bounds,(Color){10,17,28,255});
    DrawRectangleLinesEx(bounds,1,e->focused ? (Color){62,230,197,255}:(Color){35,56,76,255});
    BeginScissorMode((int)bounds.x+2,(int)bounds.y+2,(int)bounds.width-4,(int)bounds.height-4);
    size_t at=0; unsigned row=0; int y=(int)bounds.y+8;
    size_t active=app_current_line(app);
    while (row<e->top && app->source[at]) { while (app->source[at] && app->source[at]!='\n') ++at; if (app->source[at]) ++at; ++row; }
    while (y<bounds.y+bounds.height) {
        char text[512]; size_t count=0, start=at;
        while (app->source[at] && app->source[at]!='\n') { if (count<sizeof(text)-1) text[count++]=app->source[at]; ++at; }
        text[count]=0;
        if (e->select_all || active==row+1) DrawRectangle((int)bounds.x+1,y-2,(int)bounds.width-2,22,(Color){23,57,62,255});
        DrawText(TextFormat("%u",row+1),(int)bounds.x+8,y,14,(Color){102,127,151,255});
        DrawText(text,(int)bounds.x+42,y,16,(Color){220,230,244,255});
        if (e->focused && e->cursor>=start && e->cursor<=at && ((int)(GetTime()*2)%2==0)) {
            size_t col=e->cursor-start; if (col>count) col=count;
            char saved=text[col]; text[col]=0;
            int x=(int)bounds.x+42+MeasureText(text,16); text[col]=saved;
            DrawLine(x,y,x,y+18,(Color){62,230,197,255});
        }
        y+=22; ++row;
        if (!app->source[at]) break;
        ++at;
    }
    EndScissorMode();
}
