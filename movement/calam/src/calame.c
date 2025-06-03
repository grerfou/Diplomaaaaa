#include "calame.h"
#include "raylib.h"
#include <math.h>

Calame CreateCalame(Vector2 startPos) {
    return (Calame){ startPos, -PI/2, false };
}

void DrawCalame(Calame c) {
    Vector2 tip = {
        c.pos.x + cosf(c.angle) * 20,
        c.pos.y + sinf(c.angle) * 20
    };
    DrawLineEx(c.pos, tip, 4, DARKBROWN);
}

