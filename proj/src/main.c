#include <stdio.h>
#include <stdbool.h>
#include "raylib.h"
#include "argile.h"
#include "paper.h"
#include "machine.h"
#include "old.h"
#include "modern.h"
#include "esp_curl.h"


/*
 * Proj 1 --> Argile, paper
 * proj 2 --> machine
 * proj 3 --> old, modern
*/


/*
typedef enum {
    MODE_ARGILE, ==> 1
    MODE_PAPER, ==> 2
    MODE_MACHINE, ==> 3
    MODE_OLD, ==> 4
    MODE_MODERN, ==> 5
    MODE_COUNT ==> 0
} AppMode;
*/    
int main(void) {
    const int width = 1920;
    const int height = 1080;

    InitWindow(width, height, "Switch Program with Space");

    //SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    //SetWindowPosition(1920, 1);
    //SetWindowState(FLAG_WINDOW_UNDECORATED);
    //SetWindowSize(width, height);
    SetTargetFPS(60);
    
    int buttonState = 0; // variable to store the button state received from ESP32
    int previousButtonState = -1;
    
    esp_init();
/*
    bool argileLoaded = false;
    bool paperLoaded = false;
    bool machineLoaded = false;
    bool oldLoaded = false;
    bool modernLoaded = false;
*/


int frameCount=0;

    while (!WindowShouldClose()) {
       if(frameCount%10 == 0) buttonState = esp_request_response();
        frameCount ++;

        if (buttonState != previousButtonState) {
            // Un seul switch pour tout gérer
            switch (previousButtonState) {
                case 1:
                        Argile_Close();
                    break;
                case 2:
                        Paper_Close();
                    break;
                case 3:
                        Machine_Close();
                    break;
                case 4:
                      Old_Close();
                    break;
                case 5:
                      Modern_Close();
                    break;
                default:
                    break;
            }

        

            switch (buttonState) {
                case 1:
                    if (!Argile_Init("./assets/text.txt")) {
                        CloseWindow();
                        printf("Erreur : impossible de charger text.txt\n");
                        return 1;
                    }
                    break;
                case 2:
                    if (!Paper_Init("./assets/text.txt")) {
                        CloseWindow();
                        printf("Erreur : impossible de charger text.txt\n");
                        return 1;
                    }
                    break;
                case 3:
                    if (!Machine_Init("./assets/text.txt")) {
                        CloseWindow();
                        printf("Erreur : impossible de charger text.txt\n");
                        return 1;
                    }
                    break;
                case 4:
                    if (!Old_Init("./assets/text.txt")) {
                      CloseWindow();
                      printf("Erreur : impossible de charger text.txt\n");
                      return 1;
                    }
                    break;
                case 5:
                    if (!Modern_Init("assets/text.txt")){
                      CloseWindow();
                      printf("Erreur : impossible de charger text.txt\n");
                      return 1;
                    }                  
                    break;
                default:
                    break;
            }

          previousButtonState = buttonState;
        }
            BeginDrawing();
            ClearBackground(BLACK);

            switch (buttonState) {
                case 1:
                    DrawText("Current Mode: ARGILE", 10, 40, 20, DARKGREEN);
                        Argile_Draw();
                    break;
                case 2:
                    DrawText("Current Mode: PAPER", 10, 40, 20, DARKGREEN);
                        Paper_Draw();
                    break;
                case 3:
                    DrawText("Current Mode: MACHINE", 10, 40, 20, DARKGREEN);
                        Machine_Draw();
                    break;
                case 4:
                    DrawText("Current Mode: OLD", 10, 40, 20, DARKGREEN);
                      Old_Draw();
                    break;
                case 5:
                    DrawText("Current Mode: MODERN", 10, 40, 20, DARKGREEN);
                      Modern_Draw();
                    break;
                default:
                    break;
            }

          EndDrawing();

    }



    // Fermeture finale selon le mode
    switch (buttonState) {
        case 1: Argile_Close(); break;
        case 2: Paper_Close(); break;
        case 3: Machine_Close(); break;
        case 4: Old_Close(); break;
        case 5: Modern_Close(); break;
        default: break;
    }

    esp_cleanup();

    CloseWindow();
    return 0;
}

