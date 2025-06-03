#include <raylib.h>
#include <stdio.h>
#include "argile.h"

int main(void)
{


    // ---------------Window--------------------//
    
    //int width = 3840;
    int width = 1920;
    //int height = 2160;
    int height = 1080;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(width, height, "Texte animé avec alpha");
    SetWindowPosition(1920, 0);
    // Passer en fullscreen fenêtré (bordeless window)
    SetWindowState(FLAG_WINDOW_UNDECORATED);
    SetWindowSize(width, height);
  
    // ---------------Window--------------------//




    //------ Argile map settings ----------// 
    //argileTextPos = (Vector2){ 1570, 340 };
    //argileTextWidth = 200;
    //argileFontSize = 7;
    //argileLineSpacing = 0;
    //argileFadeSpeed = 1.1f;
    //argileMaxLength = 500;

    //argileAnglePitch = 1.0f;
    //argileAngleYaw = 45.0f;
    //argileTextAngle = 10.0f;
    //------ Argile map settings ----------// 




    if (!Argile_Init("./assets/text.txt")) {
        CloseWindow();
        printf("Erreur : impossible de charger text.txt\n");
        return 1;
    }

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        Argile_Draw();
        EndDrawing();
    }

    Argile_Close();
    CloseWindow();

    return 0;
}

