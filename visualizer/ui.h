#ifndef LAB_UI_H
#define LAB_UI_H
#include "scene.h"
void ui_init(void);
void ui_draw(Playback *lab, const char *program_name);
Rectangle ui_viewport(void);
#endif
