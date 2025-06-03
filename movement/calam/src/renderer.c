#include "renderer.h"
#include <math.h>

static RenderTexture2D tablet;

void InitRenderer(void) {
    tablet = LoadRenderTexture(800, 600);
    BeginTextureMode(tablet);
    ClearBackground((Color){ 222, 184, 135, 255 }); // Argile clair
    EndTextureMode();
}

void Impression(Vector2 pos, float angle) {
    BeginTextureMode(tablet);

    Vector2 end = {
        pos.x + cosf(angle) * 10,
        pos.y + sinf(angle) * 10
    };

    DrawTriangle(
        pos,
        (Vector2){pos.x + 4 * cosf(angle + PI / 2), pos.y + 4 * sinf(angle + PI / 2)},
        end,
        DARKBROWN
    );

    EndTextureMode();
}

void DrawTablet(void) {
    DrawTextureRec(tablet.texture, (Rectangle){0, 0, tablet.texture.width, -tablet.texture.height}, (Vector2){0, 0}, WHITE);
}

