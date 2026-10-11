#ifndef LAB_UI_H
#define LAB_UI_H
#include "scene.h"
#include "app.h"
void ui_init(void);
void ui_draw(LabApp *app, const char *program_name);
bool ui_editor_active(void);
void ui_reset_editor(void);
Rectangle ui_viewport(void);
#endif
