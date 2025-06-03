#include "paper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

// === Paramètres modifiables ===
int paperFontSize = 20;
int paperLineSpacing = 2;
float paperTextWidth = 250.0f;

// Coins du plan (modifiable par sliders)
Vector2 paperTopLeft = {100, 100};
Vector2 paperTopRight = {500, 100};
Vector2 paperBottomLeft = {100, 300};
Vector2 paperBottomRight = {500, 300};

// === Variables internes texte ===
static char *paperText = NULL;
static Font handwritingFont;
static bool fontLoaded = false;

// Animation écriture lettre par lettre
static int lettersDisplayed = 0;
static float letterTimer = 1.0f;
static const float LETTER_DELAY = 0.15f;

// Sliders texte
static bool showSliders = true;
static int activeSlider = -1;
static const float sliderRadius = 8.0f;

// Défilement vertical du texte
static float verticalScrollOffset = 0.0f;

// === Variables internes shader ===
// Coins shader indépendants des coins texte
static Vector2 shaderTopLeft = {600, 100};
static Vector2 shaderTopRight = {1000, 100};
static Vector2 shaderBottomLeft = {600, 300};
static Vector2 shaderBottomRight = {1000, 300};

static Shader shader = {0};
static int uniformTimeLoc = -1;
static int uniformResolutionLoc = -1;
static int uniformCornersLoc = -1;
static int uniformOffsetLoc = -1;
static int uniformScaleLoc = -1;

static Vector2 shaderOffset = {0.0f, 0.0f};
static float shaderScale = 1.0f;

static int activeSliderShader = -1;

// ==== Fonctions utilitaires ====

static bool IsMouseOverPoint(Vector2 point, Vector2 mousePos, float radius) {
    return CheckCollisionPointCircle(mousePos, point, radius);
}

// === Gestion sliders texte et shader ===
static void DrawAndHandleSliders(void) {
    Vector2 mousePos = GetMousePosition();

    // Clic initial
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Sliders texte
        if (IsMouseOverPoint(paperTopLeft, mousePos, sliderRadius)) activeSlider = 0;
        else if (IsMouseOverPoint(paperTopRight, mousePos, sliderRadius)) activeSlider = 1;
        else if (IsMouseOverPoint(paperBottomLeft, mousePos, sliderRadius)) activeSlider = 2;
        else if (IsMouseOverPoint(paperBottomRight, mousePos, sliderRadius)) activeSlider = 3;
        // Sliders shader
        else if (IsMouseOverPoint(shaderTopLeft, mousePos, sliderRadius)) activeSliderShader = 4;
        else if (IsMouseOverPoint(shaderTopRight, mousePos, sliderRadius)) activeSliderShader = 5;
        else if (IsMouseOverPoint(shaderBottomLeft, mousePos, sliderRadius)) activeSliderShader = 6;
        else if (IsMouseOverPoint(shaderBottomRight, mousePos, sliderRadius)) activeSliderShader = 7;
        else {
            activeSlider = -1;
            activeSliderShader = -1;
        }
    }
    // Relâchement clic
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        activeSlider = -1;
        activeSliderShader = -1;
    }

    // Glisser sliders texte
    if (activeSlider != -1) {
        switch (activeSlider) {
            case 0: paperTopLeft = mousePos; break;
            case 1: paperTopRight = mousePos; break;
            case 2: paperBottomLeft = mousePos; break;
            case 3: paperBottomRight = mousePos; break;
        }
    }

    // Glisser sliders shader
    if (activeSliderShader != -1) {
        switch (activeSliderShader) {
            case 4: shaderTopLeft = mousePos; break;
            case 5: shaderTopRight = mousePos; break;
            case 6: shaderBottomLeft = mousePos; break;
            case 7: shaderBottomRight = mousePos; break;
        }
    }

    if (showSliders) {
        // Dessin sliders texte (couleurs vives)
        DrawCircleV(paperTopLeft, sliderRadius, RED);
        DrawCircleV(paperTopRight, sliderRadius, GREEN);
        DrawCircleV(paperBottomLeft, sliderRadius, BLUE);
        DrawCircleV(paperBottomRight, sliderRadius, YELLOW);

        // Dessin sliders shader (gris foncé)
        DrawCircleV(shaderTopLeft, sliderRadius, DARKGRAY);
        DrawCircleV(shaderTopRight, sliderRadius, DARKGRAY);
        DrawCircleV(shaderBottomLeft, sliderRadius, DARKGRAY);
        DrawCircleV(shaderBottomRight, sliderRadius, DARKGRAY);
    }
}

// === Texte manuscrit sur plan déformé avec scrolling ===
static void DrawHandwritingTextOnPlaneScrolling(Font font, const char *str, int maxLetters, int fontSize, int spacing, Color baseColor)
{
    int lineHeight = fontSize + spacing;
    int totalDrawn = 0;
    int index = 0;

    // Découpage en lignes
    char lines[1024][512];
    int numLines = 0;

    while (str[index] && totalDrawn < maxLetters && numLines < 1024)
    {
        char line[512] = {0};
        float lineWidth = 0;
        int lineCharCount = 0;

        while (str[index] && str[index] != '\n' && totalDrawn < maxLetters)
        {
            char ch[2] = { str[index], 0 };
            float chWidth = MeasureTextEx(font, ch, fontSize, 1).x;

            if (lineWidth + chWidth > paperTextWidth) break;

            line[lineCharCount++] = str[index++];
            lineWidth += chWidth;
            totalDrawn++;
        }

        line[lineCharCount] = '\0';
        strcpy(lines[numLines++], line);

        if (str[index] == '\n') index++;
    }

    int totalTextHeight = lineHeight * numLines;

    float planeHeight = Vector2Distance(paperTopLeft, paperBottomLeft);

    // Scroll vers le haut si texte trop grand
    if (totalTextHeight > planeHeight) {
        verticalScrollOffset += (lineHeight * GetFrameTime() * 20); // vitesse scroll
        if (verticalScrollOffset > (totalTextHeight - planeHeight))
            verticalScrollOffset = totalTextHeight - planeHeight;
    }
    else {
        verticalScrollOffset = 0;
    }

    int letterIndex = 0;

    for (int i = 0; i < numLines; i++)
    {
        float tY = ((lineHeight * i) - verticalScrollOffset) / planeHeight;

        if (tY < 0 || tY > 1) continue;

        Vector2 start = Vector2Lerp(paperTopLeft, paperBottomLeft, tY);
        Vector2 end = Vector2Lerp(paperTopRight, paperBottomRight, tY);

        float cursor = 0;
        Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));

        for (int c = 0; lines[i][c]; c++)
        {
            if (letterIndex >= maxLetters) return;

            char ch[2] = { lines[i][c], 0 };
            float chWidth = MeasureTextEx(font, ch, fontSize, 1).x;

            Vector2 pos = Vector2Add(start, Vector2Scale(dir, cursor));

            DrawTextEx(font, ch, pos, fontSize, 1, baseColor);

            cursor += chWidth;
            letterIndex++;
        }
    }
}

// --- Fonction de chargement des positions ---
static void LoadPositions(void)
{
    FILE *f = fopen("assets/position/positions_paper.cfg", "r");
    if (!f) return;

    // On lit les 8 vecteurs x,y dans l'ordre
    if (fscanf(f,
               "%f %f\n%f %f\n%f %f\n%f %f\n%f %f\n%f %f\n%f %f\n%f %f\n",
               &paperTopLeft.x, &paperTopLeft.y,
               &paperTopRight.x, &paperTopRight.y,
               &paperBottomLeft.x, &paperBottomLeft.y,
               &paperBottomRight.x, &paperBottomRight.y,
               &shaderTopLeft.x, &shaderTopLeft.y,
               &shaderTopRight.x, &shaderTopRight.y,
               &shaderBottomLeft.x, &shaderBottomLeft.y,
               &shaderBottomRight.x, &shaderBottomRight.y) != 16)
    {
        // erreur ou fichier mal formé, on ne modifie rien
    }
    fclose(f);
}

// --- Nouvelle fonction de sauvegarde des positions ---
static void SavePositions(void)
{
    FILE *f = fopen("assets/position/positions_paper.cfg", "w");
    if (!f) return;

    fprintf(f, "%f %f\n", paperTopLeft.x, paperTopLeft.y);
    fprintf(f, "%f %f\n", paperTopRight.x, paperTopRight.y);
    fprintf(f, "%f %f\n", paperBottomLeft.x, paperBottomLeft.y);
    fprintf(f, "%f %f\n", paperBottomRight.x, paperBottomRight.y);

    fprintf(f, "%f %f\n", shaderTopLeft.x, shaderTopLeft.y);
    fprintf(f, "%f %f\n", shaderTopRight.x, shaderTopRight.y);
    fprintf(f, "%f %f\n", shaderBottomLeft.x, shaderBottomLeft.y);
    fprintf(f, "%f %f\n", shaderBottomRight.x, shaderBottomRight.y);

    fclose(f);
}

// === Initialisation ===
bool Paper_Init(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    paperText = (char *)malloc(size + 1);
    if (!paperText) {
        fclose(f);
        return false;
    }

    fread(paperText, 1, size, f);
    paperText[size] = '\0';
    fclose(f);

    if (FileExists("assets/typo/Satisfy-Regular.ttf")) {
        handwritingFont = LoadFont("assets/typo/Satisfy-Regular.ttf");
        fontLoaded = true;
    } else {
        handwritingFont = GetFontDefault();
    }

    lettersDisplayed = 0;
    letterTimer = 0.0f;
    verticalScrollOffset = 0.0f;

    // Charger shader argile.fs (chemin à adapter)
    shader = LoadShader(NULL, "assets/shaders/paper.fs");
    uniformTimeLoc = GetShaderLocation(shader, "time");
    uniformResolutionLoc = GetShaderLocation(shader, "resolution");
    uniformCornersLoc = GetShaderLocation(shader, "corners");
    uniformOffsetLoc = GetShaderLocation(shader, "offset");
    uniformScaleLoc = GetShaderLocation(shader, "scale");

    shaderOffset = (Vector2){0, 0};
    shaderScale = 1.0f;

    // Charger les positions sauvegardées si possible
    LoadPositions();

    return true;
}

// === Mise à jour offset shader par clavier flèches ===
static void UpdateShaderOffset(void)
{
    const float moveSpeed = 100.0f;
    bool moved = false;

    if (IsKeyDown(KEY_RIGHT)) { shaderOffset.x += moveSpeed * GetFrameTime(); moved = true; }
    if (IsKeyDown(KEY_LEFT))  { shaderOffset.x -= moveSpeed * GetFrameTime(); moved = true; }
    if (IsKeyDown(KEY_UP))    { shaderOffset.y -= moveSpeed * GetFrameTime(); moved = true; }
    if (IsKeyDown(KEY_DOWN))  { shaderOffset.y += moveSpeed * GetFrameTime(); moved = true; }

    if (moved) {
        // Debug
        // printf("Shader offset: (%f, %f)\n", shaderOffset.x, shaderOffset.y);
    }
}

// === Dessin principal ===
void Paper_Draw(void)
{
    ClearBackground(BLACK);
    if (!paperText) return;

    letterTimer += GetFrameTime();

    if (lettersDisplayed < (int)strlen(paperText) && letterTimer >= LETTER_DELAY) {
        letterTimer = 0.0f;
        lettersDisplayed++;
    }

    // Mise à jour offset shader par clavier
    UpdateShaderOffset();

    // Uniforms shader
    float timeValue = GetTime();
    Vector2 screenSize = {(float)GetScreenWidth(), (float)GetScreenHeight()};
    SetShaderValue(shader, uniformTimeLoc, &timeValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, uniformResolutionLoc, &screenSize, SHADER_UNIFORM_VEC2);

    Vector2 corners[4] = {shaderTopLeft, shaderTopRight, shaderBottomRight, shaderBottomLeft};
    SetShaderValueV(shader, uniformCornersLoc, (float *)corners, SHADER_UNIFORM_VEC2, 4);
    SetShaderValue(shader, uniformOffsetLoc, &shaderOffset, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, uniformScaleLoc, &shaderScale, SHADER_UNIFORM_FLOAT);

    // Dessiner plan shader
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

    DrawTriangle(paperTopLeft, paperBottomRight, paperTopRight, BLACK);
    DrawTriangle(paperTopLeft, paperBottomLeft, paperBottomRight, BLACK);

    // Dessiner texte au-dessus
    Color textColor = (Color){ 255, 255, 255, 255 };
    DrawHandwritingTextOnPlaneScrolling(handwritingFont, paperText, lettersDisplayed, paperFontSize, paperLineSpacing, textColor);

    // Dessiner sliders
    if (IsKeyPressed(KEY_TAB)) showSliders = !showSliders;
    DrawAndHandleSliders();

    DrawText("Press TAB to toggle sliders", 10, 10, 20, GRAY);
}

// === Nettoyage ===
void Paper_Close(void)
{
    if (paperText) {
        free(paperText);
        paperText = NULL;
    }
    if (fontLoaded) {
        UnloadFont(handwritingFont);
    }

    if (shader.id != 0) {
        UnloadShader(shader);
    }

    // Sauvegarder les positions quand on ferme le programme
    SavePositions();
}

