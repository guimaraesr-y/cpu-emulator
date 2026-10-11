#ifndef LAB_EDITOR_SCROLL_H
#define LAB_EDITOR_SCROLL_H
#include <stdbool.h>
unsigned editor_scroll(unsigned top, unsigned lines, unsigned visible,
                       unsigned cursor_line, int wheel_lines, bool reveal_cursor);
#endif
