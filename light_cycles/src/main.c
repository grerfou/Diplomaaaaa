#include "raylib.h"
#include "titan_tube.h"
#include <math.h>
#include "dmx_controls.h"
#include "esp_curl.h"

serialib serial;

const char *serial_port = "/dev/ttyUSB0";

// Déclaration des fonctions DMX
int open_dmx_port(const char *port);
void send_dmx_data(TitanTube *tubes, int num_tubes);
void close_dmx_port();

void clear_tube_cues(TitanTube *tubes, int count) {
    for (int i = 0; i < count; i++) {
        tubes[i].cue_count = 0;
        tubes[i].color = BLACK;
    }
}

int main(void) {
    const int screenWidth = 1300;
    const int screenHeight = 300;

    InitWindow(screenWidth, screenHeight, "TitanTube Cycles");

    SetTargetFPS(60);
    
    TitanTube tubes[NUM_TUBES];
    for (int i = 0; i < NUM_TUBES; i++) {
        tubes[i] = create_tube(i * 10);
    }

    const TubeCycle *cycles;
    int cycle_count = get_available_cycles(&cycles);
    int selected_cycle = 0;
    float start_time = GetTime();

    // Configure premier cycle
    cycles[selected_cycle].setup(tubes, NUM_TUBES);

    // Ouvre le port DMX
    if (open_dmx_port(&serial, serial_port) != 0) {
        CloseWindow();
        return -1;
    }

    int buttonState = 0; // variable to store the button state received from ESP32
    int previousButtonState = -1;

    esp_init();

    int frameCount=0;

    while (!WindowShouldClose()) {
     if(frameCount%10 == 0) buttonState = esp_request_response();
      frameCount ++;
   

      if (buttonState != previousButtonState) {
            previousButtonState = buttonState;
            selected_cycle = buttonState;

            clear_tube_cues(tubes, NUM_TUBES);
            cycles[selected_cycle].setup(tubes, NUM_TUBES);
            start_time = GetTime();
            printf("selected_cycle = %i", selected_cycle);
        }

        // Temps global depuis le début du cycle
        float elapsed = GetTime() - start_time;
        float duration = cycles[selected_cycle].duration;

        // Boucle sur la durée du cycle
        float local_time = fmodf(elapsed, duration);

        // Met à jour les tubes avec le temps cyclique
        for (int i = 0; i < NUM_TUBES; i++) {
            update_tube(&tubes[i], local_time);
        }


        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int i = 0; i < NUM_TUBES; i++) {
            draw_tube(&tubes[i], i);
        }

        DrawText(TextFormat("Cycle: %s", cycles[selected_cycle].name), 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("Time: %.2f (loop %.2f)", elapsed, local_time), 10, 35, 20, GRAY);
        DrawText("Espace : changer de cycle", 10, 60, 18, DARKGRAY);

        EndDrawing();
        // Envoie les données DMX
        send_dmx_data(&serial, tubes, NUM_TUBES);
    }

    // Ferme le port DMX à la fin
    close_dmx_port(&serial);
    esp_cleanup();
    CloseWindow();
    return 0;
}

