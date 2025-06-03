#include "machine.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// === Paramètres ===
int machineFontSize = 26;
float machineLineSpacing = 8;
float machineTextWidth = 240.0f;
float machineTextHeight = 700.0f;

Vector2 machineTopLeft = {100, 100};
Vector2 machineTopRight = {340, 100};
Vector2 machineBottomLeft = {100, 300};
Vector2 machineBottomRight = {340, 300};

// === Coin du shaders ===

static Vector2 shaderTopLeft = {600, 100};
static Vector2 shaderTopRight = {1000, 100};
static Vector2 shaderBottomLeft = {600, 300};
static Vector2 shaderBottomRight = {1000, 300};

// === Variables internes ===
static Font MachineFont;
static bool fontLoaded = false;
static char *machineText = NULL;

static int machineDisplayedLetters = 0;
static float machineLetterTimer = 0.0f;
static const float MACHINE_LETTER_DELAY = 0.08f;

static int machineAnimatingLetter = -1;
static float machineLetterAnimTimer = 0.0f;
static const float MACHINE_LETTER_ANIM_DURATION = 0.08f;

static float machineScrollY = 0.0f;



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
        if (IsMouseOverPoint(machineTopLeft, mousePos, sliderRadius)) activeSlider = 0;
        else if (IsMouseOverPoint(machineTopRight, mousePos, sliderRadius)) activeSlider = 1;
        else if (IsMouseOverPoint(machineBottomLeft, mousePos, sliderRadius)) activeSlider = 2;
        else if (IsMouseOverPoint(machineBottomRight, mousePos, sliderRadius)) activeSlider = 3;

        else if (IsMouseOverPoint(shaderTopLeft, mousePos, sliderRadius)) activeSlider = 4;
        else if (IsMouseOverPoint(shaderTopRight, mousePos, sliderRadius)) activeSlider = 5;
        else if (IsMouseOverPoint(shaderBottomLeft, mousePos, sliderRadius)) activeSlider = 6;
        else if (IsMouseOverPoint(shaderBottomRight, mousePos, sliderRadius)) activeSlider = 7;



        else activeSlider = -1;
    } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        activeSlider = -1;
    }

    if (activeSlider != -1) {
        switch (activeSlider) {
            case 0: machineTopLeft = mousePos; break;
            case 1: machineTopRight = mousePos; break;
            case 2: machineBottomLeft = mousePos; break;
            case 3: machineBottomRight = mousePos; break;
            case 4: shaderTopLeft = mousePos; break;
            case 5: shaderTopRight = mousePos; break;
            case 6: shaderBottomLeft = mousePos; break;
            case 7: shaderBottomRight = mousePos; break;

        }
    }

    if (showSliders) {
        DrawCircleV(machineTopLeft, sliderRadius, RED);
        DrawCircleV(machineTopRight, sliderRadius, GREEN);
        DrawCircleV(machineBottomLeft, sliderRadius, BLUE);
        DrawCircleV(machineBottomRight, sliderRadius, YELLOW);

        DrawCircleV(shaderTopLeft, sliderRadius, DARKGRAY);
        DrawCircleV(shaderTopRight, sliderRadius, DARKGRAY);
        DrawCircleV(shaderBottomLeft, sliderRadius, DARKGRAY);
        DrawCircleV(shaderBottomRight, sliderRadius, DARKGRAY);
    }
}

static Vector2 GetLetterShake(void) {
    float strength = 2.0f;
    return (Vector2){ GetRandomValue(-strength, strength), GetRandomValue(-strength, strength) };
}


static void DrawTextOnPlaneMachine(Font font, const char *str, int maxLen, int fontSize, float spacing, Color color) {
    int lineHeight = fontSize + spacing;
    int maxVisibleLines = (int)(machineTextHeight / lineHeight);

    char lines[1024][512];
    int numLines = 0;
    int index = 0;
    int totalDrawn = 0;

    // Formater le texte en lignes
    while (str[index] && totalDrawn < maxLen && numLines < 1024) {
        char line[512] = {0};
        float lineWidth = 0;
        int charCount = 0;

        while (str[index] && totalDrawn < maxLen) {
            char ch[2] = { str[index], 0 };
            float chWidth = MeasureTextEx(font, ch, fontSize, 1).x;

            if (lineWidth + chWidth > machineTextWidth) break;

            line[charCount++] = str[index++];
            lineWidth += chWidth;
            totalDrawn++;
        }

        line[charCount] = '\0';
        strcpy(lines[numLines++], line);
    }

    // Calcul du scroll pour que la ligne en cours d’écriture soit toujours en bas
    int startLine = (numLines > maxVisibleLines) ? (numLines - maxVisibleLines) : 0;

    Vector2 scrollOffset = Vector2Subtract(
        Vector2Lerp(machineTopLeft, machineBottomLeft, (float)startLine / maxVisibleLines),
        machineTopLeft
    );
    
    Vector2 tl = machineTopLeft;
    Vector2 tr = machineTopRight;
    Vector2 br = machineBottomRight;
    Vector2 bl = machineBottomLeft;
    DrawTriangle(tl, br, tr, BLACK);
    DrawTriangle(tl, bl, br, BLACK);



    int letterIndex = 0;
    for (int i = 0; i < numLines; i++) {
        float tY = (float)i / maxVisibleLines;
        Vector2 start = Vector2Lerp(machineTopLeft, machineBottomLeft, tY);
        Vector2 end = Vector2Lerp(machineTopRight, machineBottomRight, tY);

        Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));
        float cursor = 0;

        for (int c = 0; lines[i][c]; c++) {
            if (letterIndex >= maxLen) return;

            char ch[2] = { lines[i][c], 0 };
            float chWidth = MeasureTextEx(font, ch, fontSize, 1).x;

            Vector2 pos = Vector2Add(start, Vector2Scale(dir, cursor));
            pos = Vector2Subtract(pos, scrollOffset);

            if (letterIndex == machineAnimatingLetter) {
                machineLetterAnimTimer += GetFrameTime();
                float t = machineLetterAnimTimer / MACHINE_LETTER_ANIM_DURATION;
                if (t > 1.0f) t = 1.0f;

                float scale = 1.5f - 0.5f * t;
                int alpha = (int)(255 * t);

                Vector2 shake = GetLetterShake();
                pos = Vector2Add(pos, shake);

                DrawTextEx(font, ch, pos, fontSize * scale, 1, (Color){ color.r, color.g, color.b, alpha });
                cursor += chWidth * scale;
            } else {
                DrawTextEx(font, ch, pos, fontSize, 1, color);
                cursor += chWidth;
            }

            letterIndex++;
        }
    }
}

static void SavePositions(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "%f %f\n", machineTopLeft.x, machineTopLeft.y);
    fprintf(f, "%f %f\n", machineTopRight.x, machineTopRight.y);
    fprintf(f, "%f %f\n", machineBottomLeft.x, machineBottomLeft.y);
    fprintf(f, "%f %f\n", machineBottomRight.x, machineBottomRight.y);

    fprintf(f, "%f %f\n", shaderTopLeft.x, shaderTopLeft.y);
    fprintf(f, "%f %f\n", shaderTopRight.x, shaderTopRight.y);
    fprintf(f, "%f %f\n", shaderBottomLeft.x, shaderBottomLeft.y);
    fprintf(f, "%f %f\n", shaderBottomRight.x, shaderBottomRight.y);

    fclose(f);
}

static void LoadPositions(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return; // Pas de fichier, garder valeurs par défaut

    fscanf(f, "%f %f\n", &machineTopLeft.x, &machineTopLeft.y);
    fscanf(f, "%f %f\n", &machineTopRight.x, &machineTopRight.y);
    fscanf(f, "%f %f\n", &machineBottomLeft.x, &machineBottomLeft.y);
    fscanf(f, "%f %f\n", &machineBottomRight.x, &machineBottomRight.y);

    fscanf(f, "%f %f\n", &shaderTopLeft.x, &shaderTopLeft.y);
    fscanf(f, "%f %f\n", &shaderTopRight.x, &shaderTopRight.y);
    fscanf(f, "%f %f\n", &shaderBottomLeft.x, &shaderBottomLeft.y);
    fscanf(f, "%f %f\n", &shaderBottomRight.x, &shaderBottomRight.y);

    fclose(f);
}


bool Machine_Init(const char *filename) {
    // Charger positions sauvegardées si présentes
    LoadPositions("assets/position/positions_machine.cfg");

    FILE *f = fopen(filename, "r");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    machineText = (char *)malloc(size + 1);
    if (!machineText) {
        fclose(f);
        return false;
    }

    fread(machineText, 1, size, f);
    machineText[size] = '\0';
    fclose(f);

    if (FileExists("assets/typo/LiberationMono.ttf")) {
        MachineFont = LoadFont("assets/typo/LiberationMono.ttf");
        fontLoaded = true;
    } else {
        MachineFont = GetFontDefault();
    }

    machineDisplayedLetters = 0;
    machineLetterTimer = 0.0f;
    machineAnimatingLetter = -1;
    machineLetterAnimTimer = 0.0f;

    shader = LoadShader(NULL, "assets/shaders/machine.fs");
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



void Machine_Draw(void) {
    ClearBackground(BLACK);
    if (!machineText) return;

    machineLetterTimer += GetFrameTime();

    if (machineDisplayedLetters < (int)strlen(machineText) && machineLetterTimer >= MACHINE_LETTER_DELAY) {
        machineLetterTimer = 0.0f;
        machineDisplayedLetters++;
        machineAnimatingLetter = machineDisplayedLetters - 1;
        machineLetterAnimTimer = 0.0f;
    }

    // Mise à jour automatique des coins droits si pas en train de glisser
    if (activeSlider == -1) {
        Vector2 dirTop = Vector2Normalize(Vector2Subtract(machineTopRight, machineTopLeft));
        Vector2 dirBottom = Vector2Normalize(Vector2Subtract(machineBottomRight, machineBottomLeft));

        machineTopRight = Vector2Add(machineTopLeft, Vector2Scale(dirTop, machineTextWidth));
        machineBottomRight = Vector2Add(machineBottomLeft, Vector2Scale(dirBottom, machineTextWidth));
    }


    // Déplacement shaderOffset via flèches
    float delta = GetFrameTime();
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

    // Calcul de la zone du plan pour le masquage (scissor mode)
    int scissorX = (int)fminf(machineTopLeft.x, machineBottomLeft.x);
    int scissorY = (int)fminf(machineTopLeft.y, machineTopRight.y);
    int scissorWidth = (int)fabsf(machineTopRight.x - machineTopLeft.x);
    int scissorHeight = (int)fabsf(machineBottomLeft.y - machineTopLeft.y);

    BeginScissorMode(scissorX, scissorY, scissorWidth, scissorHeight);
    DrawTextOnPlaneMachine(MachineFont, machineText, machineDisplayedLetters, machineFontSize, machineLineSpacing, WHITE);
    EndScissorMode();

    // Sliders
    if (IsKeyPressed(KEY_TAB)) showSliders = !showSliders;
    DrawAndHandleSliders();
}



void Machine_Close(void) {
    SavePositions("assets/position/positions_machine.cfg");
    if (machineText) {
        free(machineText);
        machineText = NULL;
    }
    if (fontLoaded) {
        UnloadFont(MachineFont);
    }
     if (shader.id != 0) {
        UnloadShader(shader);
    }
}

