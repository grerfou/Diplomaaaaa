#include "raylib.h"
#include "rlgl.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


// === Text ===
static char *fullText = NULL;
static int currentChar = 0;
static float timer = 0.0f;
static float charDelay = 0.04f;
static Vector2 textPosition = { 50, 100 };
static bool isDone = false;
static Font retroFont;

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
static bool firstInit = true;
static float shaderScale = 1.0f;

static int activeSliderShader = -1;
static bool showSliders = true;
static const float sliderRadius = 8.0f;




//============================================== SHADER =================================//

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
        if (IsMouseOverPoint(shaderTopLeft, mousePos, sliderRadius)) activeSliderShader = 0;
        // Sliders shader
        else if (IsMouseOverPoint(shaderTopRight, mousePos, sliderRadius)) activeSliderShader = 1;
        else if (IsMouseOverPoint(shaderBottomLeft, mousePos, sliderRadius)) activeSliderShader = 2;
        else if (IsMouseOverPoint(shaderBottomRight, mousePos, sliderRadius)) activeSliderShader = 3;
        else {
            activeSliderShader = -1;
        }
    }
    // Relâchement clic
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        activeSliderShader = -1;
    }

    // Glisser sliders shader
    if (activeSliderShader != -1) {
        switch (activeSliderShader) {
            case 0: shaderTopLeft = mousePos; break;
            case 1: shaderTopRight = mousePos; break;
            case 2: shaderBottomLeft = mousePos; break;
            case 3: shaderBottomRight = mousePos; break;
        }
    }

    if (showSliders) {
        // Dessin sliders shader (gris foncé)
        DrawCircleV(shaderTopLeft, sliderRadius, DARKGRAY);
        DrawCircleV(shaderTopRight, sliderRadius, DARKGRAY);
        DrawCircleV(shaderBottomLeft, sliderRadius, DARKGRAY);
        DrawCircleV(shaderBottomRight, sliderRadius, DARKGRAY);
    }
}

// --- Fonction de chargement des positions ---
static void LoadPositions(void)
{
    FILE *f = fopen("assets/position/positions_modern.cfg", "r");
    if (!f) return;

    // On lit les 8 vecteurs x,y dans l'ordre
    if (fscanf(f,
               "%f %f\n%f %f\n%f %f\n%f %f\n%f %f\n%f %f\n%f %f\n%f %f\n",
               &shaderTopLeft.x, &shaderTopLeft.y,
               &shaderTopRight.x, &shaderTopRight.y,
               &shaderBottomLeft.x, &shaderBottomLeft.y,
               &shaderBottomRight.x, &shaderBottomRight.y) != 16)
    {
        // erreur ou fichier mal formé, on ne modifie rien
    }
    fclose(f);
}

// --- Fonction de sauvegarde des positions ---
static void SavePositions(void)
{
    FILE *f = fopen("assets/position/positions_modern.cfg", "w");
    if (!f) return;
    fprintf(f, "%f %f\n", shaderTopLeft.x, shaderTopLeft.y);
    fprintf(f, "%f %f\n", shaderTopRight.x, shaderTopRight.y);
    fprintf(f, "%f %f\n", shaderBottomLeft.x, shaderBottomLeft.y);
    fprintf(f, "%f %f\n", shaderBottomRight.x, shaderBottomRight.y);

    fclose(f);
}

//============================================== SHADER =================================//





bool Modern_Init(const char *filename)
{
    if (fullText) {
        free(fullText);
        fullText = NULL;
    }

    FILE *f = fopen(filename, "r");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    fullText = (char *)malloc(size + 1);
    if (!fullText) {
        fclose(f);
        return false;
    }

    fread(fullText, 1, size, f);
    fullText[size] = '\0';
    fclose(f);

    currentChar = 0;
    timer = 0.0f;
    isDone = false;

    retroFont = LoadFont("assets/typo/Didot.otf");
    if (retroFont.texture.id == 0) {
        retroFont = GetFontDefault();
    }

    //================================================ Shader ========================//
    // Charger shader argile.fs (chemin à adapter)
    shader = LoadShader(NULL, "assets/shaders/modern.fs");
    uniformTimeLoc = GetShaderLocation(shader, "time");
    uniformResolutionLoc = GetShaderLocation(shader, "resolution");
    uniformCornersLoc = GetShaderLocation(shader, "corners");
    uniformOffsetLoc = GetShaderLocation(shader, "offset");
    uniformScaleLoc = GetShaderLocation(shader, "scale");

    if (firstInit)
    {
      shaderOffset = (Vector2){0, 0};
      shaderScale = 1.0f;
      firstInit = false;
    }
    
    LoadPositions();

    //================================================ Shader ========================//
    return true;
}

    //================================================ Shader ========================//
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

    //================================================ Shader ========================//


void Modern_Draw()
{


    //================================================ Shader ========================//
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

    // Dessiner sliders
    if (IsKeyPressed(KEY_TAB)) showSliders = !showSliders;
    DrawAndHandleSliders();

    //================================================ Shader ========================//
    

    if (!fullText) return;
    float fontSize = 32;
    float spacing = 2;
    float lineHeight = fontSize + spacing;
    float maxWidth = GetScreenWidth() - textPosition.x - 10;
    float screenHeight = GetScreenHeight();

    Vector2 pos = textPosition;

    // Scroll vertical dynamique
    static float scrollOffsetY = 0;
    int visibleLines = (int)((screenHeight - textPosition.y) / lineHeight);
    int totalLines = 0;

    // Première passe : calculer le nombre de lignes nécessaires
    Vector2 tempPos = pos;
    for (int j = 0; j < currentChar; j++) {
        if (fullText[j] == '\n') {
            tempPos.y += lineHeight;
            tempPos.x = textPosition.x;
            totalLines++;
            continue;
        }

        char buffer[2] = { fullText[j], '\0' };
        Vector2 charSize = MeasureTextEx(retroFont, buffer, fontSize, spacing);

        if (tempPos.x + charSize.x > maxWidth) {
            tempPos.y += lineHeight;
            tempPos.x = textPosition.x;
            totalLines++;
        }

        tempPos.x += charSize.x;
    }

    // Scroll si dépasse écran
    int overflowLines = totalLines - visibleLines + 1;
    scrollOffsetY = (overflowLines > 0) ? (overflowLines * lineHeight) : 0;

    // Affichage du texte visible
    pos.y -= scrollOffsetY;
    int i = 0;
    while (i < currentChar)
    {
        if (fullText[i] == '\n') {
            pos.y += lineHeight;
            pos.x = textPosition.x;
            i++;
            continue;
        }

        char buffer[2] = { fullText[i], '\0' };
        Vector2 charSize = MeasureTextEx(retroFont, buffer, fontSize, spacing);

        if (pos.x + charSize.x > maxWidth) {
            pos.y += lineHeight;
            pos.x = textPosition.x;
        }

        if (pos.y + charSize.y > 0 && pos.y < screenHeight) {
            DrawTextEx(retroFont, buffer, pos, fontSize, spacing, WHITE);
        }

        pos.x += charSize.x;
        i++;
    }

    // Écriture progressive avec pauses naturelles
    timer += GetFrameTime();

    static int charsSincePause = 0;
    static float pauseDuration = 0.0f;

    if (pauseDuration > 0.0f) {
        pauseDuration -= GetFrameTime();
        return;
    }

    if (timer >= charDelay)
    {
        timer = 0.0f;
        currentChar++;
        charsSincePause++;

        // Pause tous les 15 à 30 caractères
        if (charsSincePause >= 15 + (rand() % 15)) {
            pauseDuration = 0.3f + ((float)(rand() % 30) / 100.0f); // 0.3 à 0.6 s
            charsSincePause = 0;
        }

        if (fullText[currentChar] == '\0') {
            currentChar = 0;
            isDone = false;
        }
    }

}



void Modern_Close()
{
    if (retroFont.texture.id != GetFontDefault().texture.id)
        UnloadFont(retroFont);

    if (fullText) {
        free(fullText);
        fullText = NULL;
    }

    //================================================ Shader ========================//
    if (shader.id != 0) {
        UnloadShader(shader);
    }
    SavePositions();
    //================================================ Shader ========================//
}

