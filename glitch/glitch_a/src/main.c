#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "raylib.h"
#include "text_loader.h"
#include "bug_letters.h"
#include "esp_curl.h"



int main(void) {
    InitWindow(1920, 1800, "Change Bug with Space");

    SetTargetFPS(60);
    
    // Chargement de la police et texte
    Font font = LoadFontEx("assets/LiberationMono.ttf", 18, NULL, 8200);


    float scrollOffset = 0.0f;
    float deltaTime = 0.1f;

    // Initialisation bugs
    InitLettersBug();

    int percentage = 0;

    ChooseWordsToScramble(percentage); // Exemple : 20% de mots buggués pour letters

    int buttonState = 0;
    int previousButtonState = -1;
    int frameCount = 0;

    esp_init();

   while (!WindowShouldClose()) {
        
      if(frameCount%10 == 0) buttonState = esp_request_response(); 
      frameCount++;

      deltaTime = GetFrameTime();

        // Gestion scroll texte simple    ------------------------------------------- A FAIRE
        scrollOffset -= 200 * deltaTime;

        // Passage au bug suivant à la touche espace (pas de répétition si maintenue)
        if (previousButtonState != buttonState){

        // Mise à jour selon bug actif
        switch (buttonState) {
            case 0:
                FreeLoadedText();
                scrollOffset = 0.0f; 
                break;
            case 1:
                percentage = 1; 
                if (!LoadTextFromFile("./assets/text.txt")) printf("Error loading text file"); //LOAD text file
                ChooseWordsToScramble(percentage);
                break;
            case 2:
                percentage = 30; 
                if (!LoadTextFromFile("./assets/text.txt")) printf("Error loading text file"); //LOAD text file
                ChooseWordsToScramble(percentage);
                break;
            case 3:
                percentage = 40; 
                if (!LoadTextFromFile("./assets/text.txt")) printf("Error loading text file"); //LOAD text file
                ChooseWordsToScramble(percentage);
                break;
            case 4:
                percentage = 70; 
                if (!LoadTextFromFile("./assets/text.txt")) printf("Error loading text file"); //LOAD text file
                ChooseWordsToScramble(percentage);
                break;
            case 5:
                percentage = 250; 
                if (!LoadTextFromFile("./assets/text.txt")) printf("Error loading text file"); //LOAD text file
                ChooseWordsToScramble(percentage);
                break;

            default:
                break;
        }
        previousButtonState = buttonState;
        }
        UpdateLettersBug(deltaTime, percentage);
        
        BeginDrawing();
        ClearBackground(BLACK);


        // Dessiner selon bug actif
        DrawLettersBug(font, scrollOffset);
        EndDrawing();
        }
    
    esp_cleanup();
    
    UnloadFont(font);
    CloseWindow();

    return 0;
}
