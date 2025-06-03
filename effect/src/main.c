#include <raylib.h>
#include <stdio.h>

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "shaders");

    Shader shader = LoadShader(0, "shaders/modern.fs");

    int resolutionLoc = GetShaderLocation(shader, "resolution");
    int timeLoc = GetShaderLocation(shader, "time");

    float resolution[2] = {(float)screenWidth, (float)screenHeight};

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        float time = GetTime();

        BeginDrawing();
        ClearBackground(BLACK);

        SetShaderValue(shader, resolutionLoc, resolution, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

        BeginBlendMode(BLEND_ALPHA);
        BeginShaderMode(shader);
        DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
        EndShaderMode();
        EndBlendMode();

        DrawFPS(10, 10);

        EndDrawing();
    }

    UnloadShader(shader);
    CloseWindow();

    return 0;
}

