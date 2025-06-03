#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <raylib.h>
#include <raylib.h>


#include <raylib.h>

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Shader mapping - cercle déformé");

    // Points pour le mapping quadrilatère
    Vector2 topLeft = {100, 100};
    Vector2 topRight = {700, 120};
    Vector2 bottomLeft = {120, 500};
    Vector2 bottomRight = {680, 480};

    Shader shader = LoadShader("shaders/test.vs", "shaders/test.fs");

    bool dragging[4] = {false, false, false, false};
    const float radius = 10.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        Vector2 mousePos = GetMousePosition();

        // Drag points
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointCircle(mousePos, topLeft, radius)) dragging[0] = true;
            else if (CheckCollisionPointCircle(mousePos, topRight, radius)) dragging[1] = true;
            else if (CheckCollisionPointCircle(mousePos, bottomLeft, radius)) dragging[2] = true;
            else if (CheckCollisionPointCircle(mousePos, bottomRight, radius)) dragging[3] = true;
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            for (int i = 0; i < 4; i++) dragging[i] = false;
        }
        for (int i = 0; i < 4; i++)
        {
            if (dragging[i]) {
                switch (i) {
                    case 0: topLeft = mousePos; break;
                    case 1: topRight = mousePos; break;
                    case 2: bottomLeft = mousePos; break;
                    case 3: bottomRight = mousePos; break;
                }
            }
        }

        // Pass uniforms au shader
        SetShaderValue(shader, GetShaderLocation(shader, "topLeft"), &topLeft, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, GetShaderLocation(shader, "topRight"), &topRight, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, GetShaderLocation(shader, "bottomLeft"), &bottomLeft, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, GetShaderLocation(shader, "bottomRight"), &bottomRight, SHADER_UNIFORM_VEC2);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginShaderMode(shader);
        DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
        EndShaderMode();

        // Affiche les points pour déplacer
        DrawCircleV(topLeft, radius, RED);
        DrawCircleV(topRight, radius, GREEN);
        DrawCircleV(bottomLeft, radius, BLUE);
        DrawCircleV(bottomRight, radius, YELLOW);

        DrawText("Glisser les points pour déformer le cercle", 10, 10, 20, WHITE);

        EndDrawing();
    }

    UnloadShader(shader);
    CloseWindow();

    return 0;
}

