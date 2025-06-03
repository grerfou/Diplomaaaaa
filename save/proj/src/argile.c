#include "argile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <rlgl.h>
#include <ctype.h>

// === Paramètres modifiables ===
int argileMaxLength = 1000; 
int argileFontSize = 10;
int argileLineSpacing = 10;
float argileFadeSpeed = 1.0f;
float argileTextWidth = 170.0f;

// Définition des coins du plan
Vector2 argileTopLeft = {100, 100};
Vector2 argileTopRight = {500, 100};
Vector2 argileBottomLeft = {100, 300};
Vector2 argileBottomRight = {500, 300};

// === Variables internes ===
static char *text = NULL;
static float alpha = 0.0f;
static int fadeDirection = 1;
static Font customFont;
static bool fontLoaded = false;

// === Sliders ===
static bool showSliders = true;
static int activeSlider = -1;
static const float sliderRadius = 8.0f;

// === Fonctions ===

// Détection si la souris est sur un point
static bool IsMouseOverPoint(Vector2 point, Vector2 mousePos, float radius) {
    return CheckCollisionPointCircle(mousePos, point, radius);
}

// Dessin et interaction sliders
static void DrawAndHandleSliders(void) {
    Vector2 mousePos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsMouseOverPoint(argileTopLeft, mousePos, sliderRadius)) activeSlider = 0;
        else if (IsMouseOverPoint(argileTopRight, mousePos, sliderRadius)) activeSlider = 1;
        else if (IsMouseOverPoint(argileBottomLeft, mousePos, sliderRadius)) activeSlider = 2;
        else if (IsMouseOverPoint(argileBottomRight, mousePos, sliderRadius)) activeSlider = 3;
        else activeSlider = -1;
    } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        activeSlider = -1;
    }

    if (activeSlider != -1) {
        switch (activeSlider) {
            case 0: argileTopLeft = mousePos; break;
            case 1: argileTopRight = mousePos; break;
            case 2: argileBottomLeft = mousePos; break;
            case 3: argileBottomRight = mousePos; break;
        }
    }

    if (showSliders) {
        DrawCircleV(argileTopLeft, sliderRadius, RED);
        DrawCircleV(argileTopRight, sliderRadius, GREEN);
        DrawCircleV(argileBottomLeft, sliderRadius, BLUE);
        DrawCircleV(argileBottomRight, sliderRadius, YELLOW);
    }
}

static void DrawTextOnPlane(Font font, const char *str, int maxLen, int fontSize, int spacing, Color tint)
{
    int lineHeight = fontSize + spacing;
    int totalDrawn = 0;
    int index = 0;

    // Découpage en lignes
    char lines[1024][512];  // max 1024 lignes, 512 caractères max par ligne
    int numLines = 0;

    while (str[index] && totalDrawn < maxLen && numLines < 1024)
    {
        char line[512] = {0};
        float lineWidth = 0;
        int lineCharCount = 0;

        while (str[index] && str[index] != '\n' && totalDrawn < maxLen)
        {
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

    // Calcul hauteur totale
    float totalHeight = lineHeight * numLines;

    for (int i = 0; i < numLines; i++)
    {
        // Calcul interpolation verticale en fonction de la hauteur de ligne et du plan
        float tY = (lineHeight * i) / totalHeight;

        // Calcul position sur les bords interpolés verticalement
        Vector2 start = Vector2Lerp(argileTopLeft, argileBottomLeft, tY);
        Vector2 end = Vector2Lerp(argileTopRight, argileBottomRight, tY);

        float cursor = 0;
        Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));

        for (int c = 0; lines[i][c]; c++)
        {
            char ch[2] = { lines[i][c], 0 };
            float chWidth = MeasureTextEx(font, ch, fontSize, 1).x;

            Vector2 pos = Vector2Add(start, Vector2Scale(dir, cursor));
            DrawTextEx(font, ch, pos, fontSize, 1, tint);
            cursor += chWidth;
        }
    }
}

// Initialisation
bool Argile_Init(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    text = (char *)malloc(size + 1);
    if (!text) {
        fclose(f);
        return false;
    }

    fread(text, 1, size, f);
    text[size] = '\0';
    fclose(f);

    if (FileExists("assets/LiberationMono.ttf")) {
        customFont = LoadFont("assets/LiberationMono.ttf");
        fontLoaded = true;
    } else {
        customFont = GetFontDefault();
    }

    return true;
}

// Dessin principal
void Argile_Draw(void) {
    ClearBackground(BLACK);
    if (!text) return;

    float delta = GetFrameTime();
    alpha += fadeDirection * argileFadeSpeed * delta;

    if (alpha >= 1.0f) {
        alpha = 1.0f;
        fadeDirection = -1;
    } else if (alpha <= 0.0f) {
        alpha = 0.0f;
        fadeDirection = 1;
    }

    // Toggle largeur avec W
    if (IsKeyPressed(KEY_W)) {
        argileTextWidth = (argileTextWidth == 160.0f) ? 260.0f : 160.0f;
    }

    // Modifier la taille de la police avec UP/DOWN
    if (IsKeyPressed(KEY_UP)) argileFontSize++;
    if (IsKeyPressed(KEY_DOWN) && argileFontSize > 1) argileFontSize--;

    // Modifier l'espacement entre les lignes avec LEFT/RIGHT
    if (IsKeyPressed(KEY_RIGHT)) argileLineSpacing++;
    if (IsKeyPressed(KEY_LEFT) && argileLineSpacing > 0) argileLineSpacing--;

    // Modifier la longueur maximale du texte avec PAGE_UP / PAGE_DOWN
    if (IsKeyPressed(KEY_PAGE_UP)) argileMaxLength += 10;
    if (IsKeyPressed(KEY_PAGE_DOWN) && argileMaxLength > 10) argileMaxLength -= 10;

    // Mise à jour auto des coins droits
    if (activeSlider == -1) {
        Vector2 dirTop = Vector2Normalize(Vector2Subtract(argileTopRight, argileTopLeft));
        Vector2 dirBottom = Vector2Normalize(Vector2Subtract(argileBottomRight, argileBottomLeft));

        argileTopRight = Vector2Add(argileTopLeft, Vector2Scale(dirTop, argileTextWidth));
        argileBottomRight = Vector2Add(argileBottomLeft, Vector2Scale(dirBottom, argileTextWidth));
    }

    Color textColor = (Color){255, 255, 255, (unsigned char)(alpha * 255)};
    DrawTextOnPlane(customFont, text, argileMaxLength, argileFontSize, argileLineSpacing, textColor);

    // Infos debug
    DrawText(TextFormat("Width: %.0f (W to toggle)", argileTextWidth), 10, 10, 20, GRAY);
    DrawText(TextFormat("FontSize: %d (UP/DOWN)", argileFontSize), 10, 40, 20, GRAY);
    DrawText(TextFormat("LineSpacing: %d (LEFT/RIGHT)", argileLineSpacing), 10, 70, 20, GRAY);
    DrawText(TextFormat("MaxLength: %d (PAGE_UP / PAGE_DOWN)", argileMaxLength), 10, 100, 20, GRAY);

    if (IsKeyPressed(KEY_TAB)) showSliders = !showSliders;
    DrawAndHandleSliders();
}

// Nettoyage mémoire
void Argile_Close(void) {
    if (text) {
        free(text);
        text = NULL;
    }
    if (fontLoaded) {
        UnloadFont(customFont);
    }
}
