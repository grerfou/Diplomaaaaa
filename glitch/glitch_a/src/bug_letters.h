#ifndef BUG_LETTERS_H
#define BUG_LETTERS_H

#include "raylib.h"

#define MAX_MOVED_WORDS 100
#define MAX_LINES 128
#define MAX_LINE_LENGTH 1024

void InitLettersBug(void);
void ChooseWordsToScramble(int percentage);
void UpdateLettersBug(float deltaTime, int percentage);
void DrawLettersBug(Font font, float scrollOffset);

#endif
