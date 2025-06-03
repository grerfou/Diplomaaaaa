#include "bug_move.h"
#include "text_loader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define BUG_MOVE_DURATION 0.1f
#define BUG_MOVE_RANGE_X  5
#define BUG_MOVE_RANGE_Y  3
#define FONT_SIZE         30
#define WORD_SPACING      10
#define TEXT_MARGIN_LEFT  50
#define TEXT_MARGIN_TOP   50
#define LINE_HEIGHT 40

Color COLOR_TEXT = {255, 255, 255, 255};
//Color COLOR_BUG = {111, 111, 111, 255};
Color COLOR_BUG = {0, 255, 255, 255};

static float bugRefreshTimer = 0.0f;
#define BUG_REFRESH_INTERVAL 0.5f
static int glitchIntensity = 50; // pourcentage de mots à bouger

typedef struct {
    int lineIndex;
    int wordIndexInLine;
    char text[64];
    Vector2 initialPosition;
    Vector2 movePosition;
    bool isMoved;
    float moveTimer;
} WordMoveInfo;

static WordMoveInfo movedWords[MAX_MOVED_WORDS];
static int movedWordsCount = 0;

// Pour stocker les mots et leurs positions une seule fois, pas à chaque frame !
typedef struct {
    int lineIndex;
    int wordIndexInLine;
    char word[64];
    Vector2 position;
} WordInfo;

static WordInfo allWords[2048];
static int totalWords = 0;

void InitGlitchBug(void) {
    movedWordsCount = 0;
    totalWords = 0;
    for (int i = 0; i < MAX_MOVED_WORDS; i++) {
        movedWords[i].isMoved = false;
        movedWords[i].moveTimer = 0.0f;
        movedWords[i].initialPosition = (Vector2){0, 0};
        movedWords[i].movePosition = (Vector2){0, 0};
        movedWords[i].text[0] = '\0';
    }

    // Remplir allWords une seule fois ici
    totalWords = 0;
    for (int i = 0; i < lineCount; i++) {
        if (!lines[i]) continue;

        char lineCopy[MAX_LINE_LENGTH];
        strncpy(lineCopy, lines[i], MAX_LINE_LENGTH);
        lineCopy[MAX_LINE_LENGTH - 1] = '\0';

        char *token = strtok(lineCopy, " \t\n.,;!?");
        float x = TEXT_MARGIN_LEFT;
        int wordIndex = 0;

        while (token && totalWords < 2048) {
            strncpy(allWords[totalWords].word, token, 63);
            allWords[totalWords].word[63] = '\0';
            allWords[totalWords].lineIndex = i;
            allWords[totalWords].wordIndexInLine = wordIndex;
            allWords[totalWords].position = (Vector2){x, TEXT_MARGIN_TOP + i * LINE_HEIGHT};
            x += MeasureText(token, FONT_SIZE) + WORD_SPACING;

            token = strtok(NULL, " \t\n.,;!?");
            wordIndex++;
            totalWords++;
        }
    }
}

void NoteGlitch(Font font, float scrollOffset) {
    (void)font;  // Paramètme inutilisé volontairement
    (void)scrollOffset;
    DrawText("Bug_01 (Move)", 100, 100, 30, RED);
}

void ChooseWordToMove(int percentage) {
    if (totalWords == 0) return;

    movedWordsCount = 0;
    int wordsToBug = (percentage * totalWords) / 100;

    for (int i = 0; i < wordsToBug && movedWordsCount < MAX_MOVED_WORDS; i++) {
        int index = rand() % totalWords;
        WordInfo *w = &allWords[index];

        // Éviter les doublons simples (optionnel)
        bool alreadyMoved = false;
        for (int j = 0; j < movedWordsCount; j++) {
            if (movedWords[j].lineIndex == w->lineIndex && movedWords[j].wordIndexInLine == w->wordIndexInLine) {
                alreadyMoved = true;
                break;
            }
        }
        if (alreadyMoved) continue;

        movedWords[movedWordsCount].lineIndex = w->lineIndex;
        movedWords[movedWordsCount].wordIndexInLine = w->wordIndexInLine;
        strncpy(movedWords[movedWordsCount].text, w->word, 63);
        movedWords[movedWordsCount].text[63] = '\0';
        movedWords[movedWordsCount].initialPosition = w->position;
        movedWords[movedWordsCount].moveTimer = 0.0f;
        movedWords[movedWordsCount].isMoved = true;

        float dx = (rand() % (BUG_MOVE_RANGE_X * 2 + 1)) - BUG_MOVE_RANGE_X;
        float dy = (rand() % (BUG_MOVE_RANGE_Y * 2 + 1)) - BUG_MOVE_RANGE_Y;
        movedWords[movedWordsCount].movePosition = (Vector2){
            w->position.x + dx,
            w->position.y + dy
        };

        movedWordsCount++;
    }
}

void UpdateGlitchBug(float deltaTime) {
    bugRefreshTimer += deltaTime;
    if (bugRefreshTimer >= BUG_REFRESH_INTERVAL) {
        bugRefreshTimer = 0.0f;
        ChooseWordToMove(glitchIntensity);
    }

    for (int i = 0; i < movedWordsCount; i++) {
        if (movedWords[i].isMoved) {
            movedWords[i].moveTimer += deltaTime;
            if (movedWords[i].moveTimer >= BUG_MOVE_DURATION) {
                movedWords[i].isMoved = false;
            }
        }
    }
}


void DrawGlitchBug(Font font, float scrollOffset) {
    // On va parcourir ligne par ligne, mot par mot
    for (int i = 0; i < lineCount; i++) {
        if (!lines[i]) continue;

        // Pour chaque ligne, découper mots (unique fois)
        char lineCopy[MAX_LINE_LENGTH];
        strncpy(lineCopy, lines[i], MAX_LINE_LENGTH);
        lineCopy[MAX_LINE_LENGTH - 1] = '\0';

        char *tokens[64] = {0};
        int tokenCount = 0;

        char *word = strtok(lineCopy, " \n");
        while (word && tokenCount < 64) {
            tokens[tokenCount++] = word;
            word = strtok(NULL, " \n");
        }

        float xPos = TEXT_MARGIN_LEFT;
        float yPos = TEXT_MARGIN_TOP + i * LINE_HEIGHT + scrollOffset;

        for (int j = 0; j < tokenCount; j++) {
            bool bugged = false;

            // Chercher si ce mot est déplacé
            for (int k = 0; k < movedWordsCount; k++) {
                if (movedWords[k].lineIndex == i && movedWords[k].wordIndexInLine == j) {
                    Vector2 pos = movedWords[k].isMoved ? movedWords[k].movePosition : movedWords[k].initialPosition;
                    pos.y += scrollOffset;
                    DrawTextEx(font, tokens[j], pos, FONT_SIZE, 2, COLOR_BUG);
                    bugged = true;
                    break;
                }
            }

            if (!bugged) {
                DrawTextEx(font, tokens[j], (Vector2){xPos, yPos}, FONT_SIZE, 2, COLOR_TEXT);
            }

            xPos += MeasureTextEx(font, tokens[j], FONT_SIZE, 2).x + WORD_SPACING;
        }
    }
}

