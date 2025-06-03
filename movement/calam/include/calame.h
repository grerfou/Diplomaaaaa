#ifndef CALAME_H
#define CALAME_H

#include "raylib.h"

typedef struct {
    Vector2 pos;
    float angle;
    bool isPressed;
} Calame;

Calame CreateCalame(Vector2 startPos);
void DrawCalame(Calame c);

#endif

