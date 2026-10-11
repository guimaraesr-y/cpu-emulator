#ifndef LAB_SCENE_H
#define LAB_SCENE_H
#include "raylib.h"
#include "playback.h"
typedef struct { Camera3D camera; float yaw, pitch, distance; } LabScene;
void scene_init(LabScene *scene);
void scene_input(LabScene *scene, Rectangle viewport);
void scene_draw(const LabScene *scene, const Playback *lab);
void scene_labels(const LabScene *scene, const Playback *lab, Rectangle viewport);
#endif
