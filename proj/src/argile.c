// argile.c
#include "argile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <math.h>

// === Paramètres modifiables ===
int argileMaxLength = 1000;
int argileFontSize = 10;
int argileLineSpacing = 10;
float argileFadeSpeed = 1.0f;
float argileTextWidth = 150.0f;

// === Coins du texte ===
Vector2 argileTopLeft = {100, 100};
Vector2 argileTopRight = {500, 100};
Vector2 argileBottomLeft = {100, 300};
Vector2 argileBottomRight = {500, 300};

// === Coins du shader ===
static Vector2 shaderTopLeft = {600, 100};
static Vector2 shaderTopRight = {1000, 100};
static Vector2 shaderBottomLeft = {600, 300};
static Vector2 shaderBottomRight = {1000, 300};

// === Variables internes ===
static char *text = NULL;
static float alpha = 0.0f;
static int fadeDirection = 1;
static Font customFont;
static bool fontLoaded = false;

static Shader shader = {0};
static int uniformTimeLoc = -1;
static int uniformResolutionLoc = -1;
static int uniformCornersLoc = -1;
static int uniformOffsetLoc = -1;
static int uniformScaleLoc = -1;

static Vector2 shaderOffset = {0.0f, 0.0f};
static float shaderScale = 1.0f;

static bool showSliders = true;
static int activeSlider = -1;
static const float sliderRadius = 8.0f;
static bool firstInit = true;

static float scrollOffset = 0.0f;
static float scrollSpeed = 30.0f;

// === Variables pour drag & drop global ===
static bool isDraggingTextQuad = false;
static Vector2 lastMousePosText = {0};
static bool isDraggingShaderQuad = false;
static Vector2 lastMousePosShader = {0};


// --- Fonction pour vérifier si un point est dans un quadrilatère ---
static bool PointInQuad(Vector2 p, Vector2 a, Vector2 b, Vector2 c, Vector2 d) {
    float cross1 = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
    float cross2 = (c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x);
    float cross3 = (d.x - c.x) * (p.y - c.y) - (d.y - c.y) * (p.x - c.x);
    float cross4 = (a.x - d.x) * (p.y - d.y) - (a.y - d.y) * (p.x - d.x);

    return (cross1 >= 0 && cross2 >= 0 && cross3 >= 0 && cross4 >= 0) ||
           (cross1 <= 0 && cross2 <= 0 && cross3 <= 0 && cross4 <= 0);
}

static bool IsMouseOverPoint(Vector2 point, Vector2 mousePos, float radius) {
    return CheckCollisionPointCircle(mousePos, point, radius);
}

static void DrawAndHandleSliders(void) {
    Vector2 mousePos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsMouseOverPoint(argileTopLeft, mousePos, sliderRadius)) activeSlider = 0;
        else if (IsMouseOverPoint(argileTopRight, mousePos, sliderRadius)) activeSlider = 1;
        else if (IsMouseOverPoint(argileBottomLeft, mousePos, sliderRadius)) activeSlider = 2;
        else if (IsMouseOverPoint(argileBottomRight, mousePos, sliderRadius)) activeSlider = 3;
        else if (IsMouseOverPoint(shaderTopLeft, mousePos, sliderRadius)) activeSlider = 4;
        else if (IsMouseOverPoint(shaderTopRight, mousePos, sliderRadius)) activeSlider = 5;
        else if (IsMouseOverPoint(shaderBottomLeft, mousePos, sliderRadius)) activeSlider = 6;
        else if (IsMouseOverPoint(shaderBottomRight, mousePos, sliderRadius)) activeSlider = 7;
        else activeSlider = -1;
    } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        activeSlider = -1;
    }

    if (activeSlider != -1) {
        Vector2 mp = mousePos;
        switch (activeSlider) {
            case 0: argileTopLeft = mp; break;
            case 1: argileTopRight = mp; break;
            case 2: argileBottomLeft = mp; break;
            case 3: argileBottomRight = mp; break;
            case 4: shaderTopLeft = mp; break;
            case 5: shaderTopRight = mp; break;
            case 6: shaderBottomLeft = mp; break;
            case 7: shaderBottomRight = mp; break;
        }
    }

    if (showSliders) {
        DrawCircleV(argileTopLeft, sliderRadius, RED);
        DrawCircleV(argileTopRight, sliderRadius, GREEN);
        DrawCircleV(argileBottomLeft, sliderRadius, BLUE);
        DrawCircleV(argileBottomRight, sliderRadius, YELLOW);

        DrawCircleV(shaderTopLeft, sliderRadius, DARKGRAY);
        DrawCircleV(shaderTopRight, sliderRadius, DARKGRAY);
        DrawCircleV(shaderBottomLeft, sliderRadius, DARKGRAY);
        DrawCircleV(shaderBottomRight, sliderRadius, DARKGRAY);
    }
}

static void DrawTextOnPlane(Font font, const char *str, int maxLen, int fontSize, int spacing, Color tint) {
    int lineHeight = fontSize + spacing;
    int totalDrawn = 0;
    int index = 0;
    char lines[1024][512] = {{0}};
    int numLines = 0;

    while (str[index] && totalDrawn < maxLen && numLines < 1024) {
        char line[512] = {0};
        float lineWidth = 0;
        int lineCharCount = 0;

        while (str[index] && str[index] != '\n' && totalDrawn < maxLen) {
            char ch[2] = { str[index], 0 };
            float chWidth = MeasureTextEx(font, ch, fontSize, 1).x;
            if (lineWidth + chWidth > argileTextWidth) break;
            line[lineCharCount++] = str[index++];
            lineWidth += chWidth;
            totalDrawn++;
        }

        line[lineCharCount] = '\0';
        strcpy(lines[numLines++], line);
        if (str[index] == '\n') index++;
    }

    float totalHeight = lineHeight * numLines;
    if (scrollOffset > totalHeight + 50) scrollOffset = 0.0f;

    // Dessine un fond noir
    Vector2 tl = argileTopLeft;
    Vector2 tr = argileTopRight;
    Vector2 br = argileBottomRight;
    Vector2 bl = argileBottomLeft;
    DrawTriangle(tl, br, tr, BLACK);
    DrawTriangle(tl, bl, br, BLACK);

    for (int i = 0; i < numLines; i++) {
        float tY = (lineHeight * i - scrollOffset) / totalHeight;
        if (tY < 0 || tY > 1) continue;

        Vector2 start = Vector2Lerp(argileTopLeft, argileBottomLeft, tY);
        Vector2 end = Vector2Lerp(argileTopRight, argileBottomRight, tY);
        float cursor = 0;
        Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));

        for (int c = 0; lines[i][c]; c++) {
            char ch[2] = { lines[i][c], 0 };
            float chWidth = MeasureTextEx(font, ch, fontSize, 1).x;
            Vector2 pos = Vector2Add(start, Vector2Scale(dir, cursor));
            DrawTextEx(font, ch, pos, fontSize, 1, tint);
            cursor += chWidth;
        }
    }
}

static void SavePositions(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "%f %f\n", argileTopLeft.x, argileTopLeft.y);
    fprintf(f, "%f %f\n", argileTopRight.x, argileTopRight.y);
    fprintf(f, "%f %f\n", argileBottomLeft.x, argileBottomLeft.y);
    fprintf(f, "%f %f\n", argileBottomRight.x, argileBottomRight.y);

    fprintf(f, "%f %f\n", shaderTopLeft.x, shaderTopLeft.y);
    fprintf(f, "%f %f\n", shaderTopRight.x, shaderTopRight.y);
    fprintf(f, "%f %f\n", shaderBottomLeft.x, shaderBottomLeft.y);
    fprintf(f, "%f %f\n", shaderBottomRight.x, shaderBottomRight.y);

    fclose(f);
}

static void LoadPositions(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return; // Pas de fichier, garder valeurs par défaut

    fscanf(f, "%f %f\n", &argileTopLeft.x, &argileTopLeft.y);
    fscanf(f, "%f %f\n", &argileTopRight.x, &argileTopRight.y);
    fscanf(f, "%f %f\n", &argileBottomLeft.x, &argileBottomLeft.y);
    fscanf(f, "%f %f\n", &argileBottomRight.x, &argileBottomRight.y);

    fscanf(f, "%f %f\n", &shaderTopLeft.x, &shaderTopLeft.y);
    fscanf(f, "%f %f\n", &shaderTopRight.x, &shaderTopRight.y);
    fscanf(f, "%f %f\n", &shaderBottomLeft.x, &shaderBottomLeft.y);
    fscanf(f, "%f %f\n", &shaderBottomRight.x, &shaderBottomRight.y);

    fclose(f);
}

bool Argile_Init(const char *filename) {
    // Charger positions sauvegardées si présentes
    LoadPositions("assets/position/positions_argile.cfg");

    FILE *f = fopen(filename, "r");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    text = (char *)malloc(size + 1);
    if (!text) { fclose(f); return false; }
    fread(text, 1, size, f);
    text[size] = '\0';
    fclose(f);

    if (FileExists("assets/LiberationMono.ttf")) {
        customFont = LoadFont("assets/LiberationMono.ttf");
        fontLoaded = true;
    } else {
        customFont = GetFontDefault();
    }

    shader = LoadShader(NULL, "assets/shaders/argile.fs");
    uniformTimeLoc = GetShaderLocation(shader, "time");
    uniformResolutionLoc = GetShaderLocation(shader, "resolution");
    uniformCornersLoc = GetShaderLocation(shader, "corners");
    uniformOffsetLoc = GetShaderLocation(shader, "offset");
    uniformScaleLoc = GetShaderLocation(shader, "scale");

    if (firstInit) {
        shaderOffset = (Vector2){0, 0};
        shaderScale = 1.0f;
        firstInit = false;
    }
    return true;
}

void Argile_Draw(void) {
    if (!text) return;
    float delta = GetFrameTime();

    alpha += fadeDirection * argileFadeSpeed * delta;
    if (alpha >= 1.0f) { alpha = 1.0f; fadeDirection = -1; }
    else if (alpha <= 0.0f) { alpha = 0.0f; fadeDirection = 1; }

    scrollOffset += scrollSpeed * delta;

    // Déplacement shaderOffset via flèches
    const float moveSpeed = 100.0f; // pixels par seconde
    bool moved = false;
    if (IsKeyDown(KEY_RIGHT)) { shaderOffset.x += moveSpeed * delta; moved = true; }
    if (IsKeyDown(KEY_LEFT))  { shaderOffset.x -= moveSpeed * delta; moved = true; }
    if (IsKeyDown(KEY_UP))    { shaderOffset.y -= moveSpeed * delta; moved = true; }
    if (IsKeyDown(KEY_DOWN))  { shaderOffset.y += moveSpeed * delta; moved = true; }

    if (moved) {
        printf("shaderOffset = (%f, %f)\n", shaderOffset.x, shaderOffset.y);
    }

    float timeValue = GetTime();
    Vector2 screenSize = { (float)GetScreenWidth(), (float)GetScreenHeight() };
    SetShaderValue(shader, uniformTimeLoc, &timeValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, uniformResolutionLoc, &screenSize, SHADER_UNIFORM_VEC2);

    Vector2 corners[4] = {shaderTopLeft, shaderTopRight, shaderBottomRight, shaderBottomLeft};
    SetShaderValueV(shader, uniformCornersLoc, (float *)corners, SHADER_UNIFORM_VEC2, 4);
    SetShaderValue(shader, uniformOffsetLoc, &shaderOffset, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, uniformScaleLoc, &shaderScale, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(shader);
    rlBegin(RL_TRIANGLES);
    rlColor4ub(255, 255, 255, 255);
    rlTexCoord2f(0.0f, 0.0f); rlVertex2f(shaderTopLeft.x, shaderTopLeft.y);
    rlTexCoord2f(1.0f, 0.0f); rlVertex2f(shaderTopRight.x, shaderTopRight.y);
    rlTexCoord2f(1.0f, 1.0f); rlVertex2f(shaderBottomRight.x, shaderBottomRight.y);

    rlTexCoord2f(0.0f, 0.0f); rlVertex2f(shaderTopLeft.x, shaderTopLeft.y);
    rlTexCoord2f(1.0f, 1.0f); rlVertex2f(shaderBottomRight.x, shaderBottomRight.y);
    rlTexCoord2f(0.0f, 1.0f); rlVertex2f(shaderBottomLeft.x, shaderBottomLeft.y);
    rlEnd();
    EndShaderMode();

    DrawTextOnPlane(fontLoaded ? customFont : GetFontDefault(), text, argileMaxLength, argileFontSize, argileLineSpacing, Fade(WHITE, alpha));

    DrawAndHandleSliders();

    // Toggle sliders avec espace
    if (IsKeyPressed(KEY_TAB)) showSliders = !showSliders;
}

void Argile_Close(void) {
    SavePositions("assets/position/positions_argile.cfg");

    if (text) {
        free(text);
        text = NULL;
    }
    if (fontLoaded) {
        UnloadFont(customFont);
        fontLoaded = false;
    }
    if (shader.id != 0) {
        UnloadShader(shader);
    }
}

