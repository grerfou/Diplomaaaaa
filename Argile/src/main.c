#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/*
 *     Pourquoi pas chaque signe en 3D ? 
 *     Ou illuster de este de l'ecriture / de l'outils ?
 *     Gravure de l'outil dans la matiere <--> chaque trace laisser --> Quel Sens ?
 *     Temps de vagabondage de penssée sur l'ecriture d'un signe.
*/

// Fonction personnalisée pour dessiner du texte avec retour à la ligne automatique
void DrawWrappedAnimatedText(Font font, const char *text, Rectangle rec, float fontSize, float spacing, Color tint)
{
    size_t textLen = strlen(text);
    char *buffer = malloc(textLen + 1);
    if (!buffer) return;
    buffer[0] = '\0';

    const char *wordStart = text;
    float y = rec.y;

    char letter[8];  // Assurez-vous que cette taille est suffisante pour les caractères UTF-8

    // Parcours le texte caractère par caractère
    for (const char *p = text; ; p++)
    {
        // Si on rencontre un espace, un saut de ligne ou la fin du texte
        if (*p == ' ' || *p == '\n' || *p == '\0')
        {
            int wordLen = p - wordStart;
            // Allouer et copier le mot
            char *word = malloc(wordLen + 1);
            strncpy(word, wordStart, wordLen);
            word[wordLen] = '\0';

            // Construire une chaîne de test qui sera la ligne courante + le mot à ajouter (avec espace si besoin)
            size_t newLineLen = strlen(buffer) + (buffer[0] != '\0' ? 1 : 0) + strlen(word) + 1;
            char *testLine = malloc(newLineLen);
            if (buffer[0] != '\0')
                sprintf(testLine, "%s %s", buffer, word);
            else
                sprintf(testLine, "%s", word);

            // Mesurer la largeur de la nouvelle ligne
            Vector2 measure = MeasureTextEx(font, testLine, fontSize, spacing);

            // Si la largeur dépasse celle du rectangle, dessiner la ligne courante et recommencer
            if (measure.x > rec.width && buffer[0] != '\0')
            {
                DrawTextEx(font, buffer, (Vector2){ rec.x, y }, fontSize, spacing, tint);
                y += fontSize + spacing;
                // Commencer une nouvelle ligne avec le mot courant
                strcpy(buffer, word);
            }
            else
            {
                // Sinon, ajouter le mot à la ligne courante
                if (buffer[0] != '\0')
                {
                    strcat(buffer, " ");
                }
                strcat(buffer, word);
            }
            free(word);
            free(testLine);

            if (*p == '\n')  // Saut de ligne explicite
            {
                DrawTextEx(font, buffer, (Vector2){ rec.x, y }, fontSize, spacing, tint);
                y += fontSize + spacing;
                buffer[0] = '\0';  // Réinitialiser la ligne courante
            }

            if (*p == '\0') break;  // Fin du texte

            // Passer aux caractères après les espaces éventuels
            while(*(p+1) == ' ') p++;
            wordStart = p + 1;
        }
    }
    // Dessiner la dernière ligne si non vide
    if (buffer[0] != '\0')
    {
        DrawTextEx(font, buffer, (Vector2){ rec.x, y }, fontSize, spacing, tint);
    }
    free(buffer);
}

int main(void)
{
    // Exécute le programme en plein écran
    //SetConfigFlags(FLAG_FULLSCREEN_MODE);

    // Taille initiale (sera remplacée par la résolution réelle en plein écran)
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "Affichage de texte Old Persian");

    // Récupère la résolution actuelle
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    // --- Chargement de la police pour Old Persian ---
    // Plage de codepoints pour Old Persian : U+103A0 à U+103DF
    int startCP = 0x103A0;
    int endCP = 0x103DF;
    int glyphCount = endCP - startCP + 1;

    // Génère le tableau de codepoints
    int *fontChars = malloc(glyphCount * sizeof(int));
    if (!fontChars)
    {
        CloseWindow();
        return 1;
    }
    for (int cp = startCP; cp <= endCP; cp++)
    {
        fontChars[cp - startCP] = cp;
    }

    // Charge la police contenant les glyphes Old Persian.
    // Assurez-vous que "NotoSansOldPersian-Regular.ttf" est bien placé dans le dossier assets.
    Font oldPersianFont = LoadFontEx("assets/NotoSansOldPersian-Regular.ttf", 48, fontChars, glyphCount);
    free(fontChars);

    // Texte Old Persian à afficher (encodé en UTF-8)
    const char *oldPersianText = " 𐎢𐎴𐎣𐎠𐎭𐎙 𐎴𐎣𐎾     𐎠𐎩𐎢𐏃𐎭𐎠 𐎯𐎨𐎠𐎭𐎺 𐎲𐎡𐎨𐎼 𐎢𐎴𐎯    𐎤𐎺𐎭𐎠 𐎺𐎡𐎱𐎀 𐎴𐎡𐎺 𐎯𐎙𐎹𐎢 𐎠𐎭 𐎠𐎧𐎡𐎹     𐎧𐎴𐎡𐎠 𐎲𐎧𐎣𐎞𐎺 𐎠𐎦𐎰𐎴𐎄𐎱     𐎡𐎹𐎡𐎭𐎴 𐎮𐎠𐎡𐎶 𐎲𐎢𐎭𐎺 𐎭𐎠𐎴𐎱 𐎨𐎬𐎰𐎣𐎴𐎷      𐎢𐎡𐎢𐎸𐍈𐏐 𐎧𐎴𐎡𐎠𐎼𐎵 𐎠𐎴 𐎢𐎮𐎪𐎧𐎶𐎡𐎰 𐎣𐎦𐎾𐎭𐎠𐎴 𐎾𐎶𐎮𐎱𐏑 𐎠𐏃𐎤𐎖𐎡      𐏃𐎠𐎭𐎴𐎺 𐎦𐎡𐎬, 𐎰𐎥𐎠𐎪𐎨𐎠 𐎲𐎪𐎠𐎹 𐎨𐎶𐎮𐎨𐎦𐎾𐎡 𐎤𐏃 𐎭 𐏃𐎬𐎢𐎶𐎭𐎠𐎱 𐎨𐎡𐎽𐎷𐎼𐎢𐎩𐎠𐎺. ";

    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Définir la zone d'affichage avec un padding de 20 pixels
        Rectangle rec = { 20, 20, width - 40, height - 40 };

        // Appel de notre fonction personnalisée pour le word wrap et animation de l'affichage
        DrawWrappedAnimatedText(oldPersianFont, oldPersianText, rec, oldPersianFont.baseSize, 2, BLACK);

        EndDrawing();
    }

    UnloadFont(oldPersianFont);
    CloseWindow();
    
    return 0;
}

