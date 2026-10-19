#ifndef LAB_EDITOR_H
#define LAB_EDITOR_H
#include "app.h"
#include "raylib.h"
typedef struct { size_t cursor; unsigned top; bool focused, select_all; } LabEditor;
void editor_draw(LabEditor *editor, LabApp *app, Rectangle bounds);
#endif
