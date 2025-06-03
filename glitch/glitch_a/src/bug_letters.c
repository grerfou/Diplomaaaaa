#include "bug_letters.h"
#include "text_loader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define FONT_SIZE 30
#define WORD_SPACING 10
#define TEXT_MARGIN_LEFT 500
#define TEXT_MARGIN_TOP 0
#define LINE_HEIGHT 40
#define MAX_MOVED_WORDS 10000

static Color COLOR_TEXT = {255, 255, 255, 255};
static Color COLOR_BUG = {0, 255, 255, 255};

static float changeInterval = 5.0f;  // toutes les 5 secondes, on change les mots buggués
static float timeSinceLastChange = 0.0f;

typedef struct {
    int lineIndex;
    int wordIndexInLine;
    char originalWord[64];
    char displayedWord[64];
    float glitchTimer;      // temps écoulé depuis le dernier changement
    float glitchInterval;   // intervalle aléatoire pour changement de lettre
    int glitchCharIndex;    // position dans le mot qui est "glitchée"
    bool active;
} WordScrambleInfo;

static WordScrambleInfo scrambledWords[MAX_MOVED_WORDS];
static int scrambledWordsCount = 0;

void InitLettersBug(void) {
    scrambledWordsCount = 0;
    for (int i = 0; i < MAX_MOVED_WORDS; i++) {
        scrambledWords[i].active = false;
        scrambledWords[i].glitchTimer = 0.0f;
        scrambledWords[i].glitchInterval = 0.1f + (float)(rand() % 100) / 1000.0f;
        scrambledWords[i].glitchCharIndex = 0;
        scrambledWords[i].originalWord[0] = '\0';
        scrambledWords[i].displayedWord[0] = '\0';
    }
}

static char RandomGlitchChar(void) {
    const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int idx = rand() % (sizeof(chars) - 1);
    return chars[idx];
}

void ChooseWordsToScramble(int percentage) {
    scrambledWordsCount = 0;

    typedef struct {
        int lineIndex;
        int wordIndexInLine;
        char word[64];
    } WordInfo;

    WordInfo allWords[2048];
    int totalWords = 0;

    for (int i = 0; i < lineCount; i++) {
        char lineCopy[MAX_LINE_LENGTH];
        strncpy(lineCopy, lines[i], MAX_LINE_LENGTH);
        lineCopy[MAX_LINE_LENGTH - 1] = '\0';

        char *token = strtok(lineCopy, " \t\n.,;!?");
        int wordIndex = 0;

        while (token && totalWords < 2048) {
            strncpy(allWords[totalWords].word, token, 63);
            allWords[totalWords].word[63] = '\0';
            allWords[totalWords].lineIndex = i;
            allWords[totalWords].wordIndexInLine = wordIndex;

            wordIndex++;
            totalWords++;
            token = strtok(NULL, " \t\n.,;!?");
      }
    }

    int wordsToScramble = int(float(percentage * totalWords)/100.);

    for (int i = 0; i < wordsToScramble && scrambledWordsCount < MAX_MOVED_WORDS; i++) {
        int idx = rand() % totalWords;
        WordInfo w = allWords[idx];

        scrambledWords[scrambledWordsCount].lineIndex = w.lineIndex;
        scrambledWords[scrambledWordsCount].wordIndexInLine = w.wordIndexInLine;
        
        strncpy(scrambledWords[scrambledWordsCount].originalWord, w.word, 63);
        scrambledWords[scrambledWordsCount].originalWord[63] = '\0';
        strncpy(scrambledWords[scrambledWordsCount].displayedWord, w.word, 63);
        scrambledWords[scrambledWordsCount].displayedWord[63] = '\0';

        scrambledWords[scrambledWordsCount].active = true;
        scrambledWords[scrambledWordsCount].glitchTimer = 0.0f;
        scrambledWords[scrambledWordsCount].glitchInterval = 0.1f + (float)(rand() % 100) / 1000.0f;
        scrambledWords[scrambledWordsCount].glitchCharIndex = 0;

        scrambledWordsCount++;
    }
}

void UpdateLettersBug(float deltaTime, int percentage) {
    timeSinceLastChange += deltaTime;
    if (timeSinceLastChange >= changeInterval) {
        timeSinceLastChange = 0.0f;
        ChooseWordsToScramble(percentage);  // On change la sélection
    }

    for (int i = 0; i < scrambledWordsCount; i++) {
        if (!scrambledWords[i].active) continue;

        scrambledWords[i].glitchTimer += deltaTime;
        if (scrambledWords[i].glitchTimer >= scrambledWords[i].glitchInterval) {
            scrambledWords[i].glitchTimer = 0.0f;

            int len = (int)strlen(scrambledWords[i].originalWord);
            if (len > 0) {
                scrambledWords[i].glitchCharIndex = (scrambledWords[i].glitchCharIndex + 1) % len;

                strncpy(scrambledWords[i].displayedWord, scrambledWords[i].originalWord, 63);
                scrambledWords[i].displayedWord[63] = '\0';

                scrambledWords[i].displayedWord[scrambledWords[i].glitchCharIndex] = RandomGlitchChar();
            }
        }
    }
}


void DrawLettersBug(Font font, float scrollOffset) {
    for (int i = 0; i < lineCount; i++) {
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
            // Chercher si le mot est buggué
            for (int k = 0; k < scrambledWordsCount; k++) {
                if (scrambledWords[k].lineIndex == i && scrambledWords[k].wordIndexInLine == j && scrambledWords[k].active) {
                    Vector2 pos = {xPos, yPos};
                    DrawTextEx(font, scrambledWords[k].displayedWord, pos, FONT_SIZE, 2, COLOR_BUG);
                    bugged = true;
                    break;
                }
            }

            if (!bugged) {
                Vector2 pos = {xPos, yPos};
                DrawTextEx(font, tokens[j], pos, FONT_SIZE, 2, COLOR_TEXT);
            }

            xPos += MeasureTextEx(font, tokens[j], FONT_SIZE, 2).x + WORD_SPACING;
        }
    }
}

