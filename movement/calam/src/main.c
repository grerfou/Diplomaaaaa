#include "raylib.h"
#include "raymath.h"
#include <math.h>

#define MAX_POINTS 1000 // Nombre maximal de points pour la trace

// Définition d'un symbole cunéiforme Old Persian simple (série de points et directions)
typedef struct {
    Vector3 position;
    Vector3 direction;
    float duration;  // Durée pour parcourir la section du symbole
} CalameStroke;

// Tableau des symboles (exemple simple)
CalameStroke symbolA[] = {
    { (Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, -1.0f }, 2.0f },
    { (Vector3){ 1.0f, 0.0f, 1.0f }, (Vector3){ -1.0f, 0.0f, 1.0f }, 2.0f },
    { (Vector3){ -1.0f, 0.0f, 1.0f }, (Vector3){ 1.0f, 0.0f, 1.0f }, 2.0f }
};

// Tableau des points laissés par le calame (clous)
Vector3 drawnPoints[MAX_POINTS];
int pointCount = 0;

// Fonction pour dessiner le calame (simple sphère pour représenter le calame)
void DrawCalame3D(CalameStroke stroke, float t) {
    // Calcul de la position du calame à un instant t
    Vector3 pos = Vector3Lerp(stroke.position, 
        (Vector3){ stroke.position.x + stroke.direction.x * t, 
                  stroke.position.y + stroke.direction.y * t, 
                  stroke.position.z + stroke.direction.z * t }, 1.0f);
    
    // Dessiner une sphère pour représenter le calame
    DrawSphere(pos, 0.05f, DARKBROWN);
    
    // Ajouter la position actuelle du calame à la trace
    if (pointCount < MAX_POINTS) {
        drawnPoints[pointCount++] = pos;
    }
}

// Dessiner la trace du calame en connectant les points laissés
void DrawTrace() {
    for (int i = 1; i < pointCount; i++) {
        // Relier les points par une ligne (représentant la trace)
        DrawLine3D(drawnPoints[i - 1], drawnPoints[i], GRAY);
    }
}

// Dessiner la surface de l'argile (un simple plan)
void DrawClaySurface() {
    // Dessiner une grande surface pour simuler l'argile
    Vector3 p1 = (Vector3){ -5.0f, -0.1f, -5.0f };
    Vector3 p2 = (Vector3){  5.0f, -0.1f, -5.0f };
    Vector3 p3 = (Vector3){  5.0f, -0.1f,  5.0f };
    Vector3 p4 = (Vector3){ -5.0f, -0.1f,  5.0f };

    // Dessiner la surface de l'argile avec un quadrilatère
    DrawTriangle3D(p1, p2, p3, DARKGRAY);
    DrawTriangle3D(p1, p3, p4, DARKGRAY);
}

void HandleZoom(Camera3D *camera) {
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        camera->position = Vector3Add(camera->position, Vector3Scale(Vector3Normalize(Vector3Subtract(camera->target, camera->position)), wheel * 0.2f));
    }
}

int main(void) {
    // Initialisation de la fenêtre et de la caméra
    InitWindow(1000, 800, "Calame 3D - Tracé Cunéiforme");
    SetTargetFPS(60);

    Camera3D camera = {
        .position = (Vector3){ 3.0f, 3.0f, 6.0f },
        .target = (Vector3){ 0.0f, 0.0f, 0.0f },
        .up = (Vector3){ 0.0f, 1.0f, 0.0f },
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    int currentStroke = 0;
    float strokeTimer = 0.0f;

    // Boucle principale de dessin
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        strokeTimer += dt;

        // Récupérer le segment actuel du symbole
        CalameStroke from = symbolA[currentStroke];
        CalameStroke to = symbolA[(currentStroke + 1) % (sizeof(symbolA) / sizeof(symbolA[0]))];

        // Calcul de l'intervalle de temps et interpolation des positions du calame
        float t = strokeTimer / from.duration;
        if (t > 1.0f) {
            strokeTimer = 0.0f;
            currentStroke = (currentStroke + 1) % (sizeof(symbolA) / sizeof(symbolA[0]));
            t = 0.0f;
        }

        // Déplacer le calame et dessiner la trace
        DrawCalame3D(from, t);
        
        // Gérer le zoom avec la molette de la souris
        HandleZoom(&camera);

        // Déplacer la caméra avec le clic droit
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Commencer à dessiner en 3D
        BeginMode3D(camera);

        // Dessiner la surface de l'argile
        DrawClaySurface();
        
        // Dessiner la trace du calame
        DrawTrace();

        // Fin du mode 3D
        EndMode3D();

        // Afficher des informations à l'écran
        DrawText("🖱️ Maintiens clic droit pour déplacer la caméra", 20, 20, 20, DARKGRAY);
        DrawText("Utilise la molette pour zoomer", 20, 50, 20, DARKGRAY);
        DrawFPS(20, 80);

        EndDrawing();
    }

    // Fermer la fenêtre et nettoyer les ressources
    CloseWindow();
    return 0;
}

