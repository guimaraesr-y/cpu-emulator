#include "editor_scroll.h"

unsigned editor_scroll(unsigned top, unsigned lines, unsigned visible,
                       unsigned cursor_line, int wheel_lines, bool reveal_cursor) {
    if (!lines) lines=1;
    if (!visible) visible=1;
    long next=(long)top+wheel_lines;
    if (next<0) next=0;
    if ((unsigned long)next>=lines) next=(long)lines-1;
    top=(unsigned)next;
    if (reveal_cursor) {
        if (cursor_line>=lines) cursor_line=lines-1;
        if (cursor_line<top) top=cursor_line;
        else if (cursor_line-top>=visible) top=cursor_line-visible+1;
    }
    return top;
}
