#ifndef TEXT_LOADER_H
#define TEXT_LOADER_H

#define MAX_LINES 128
#define MAX_LINE_LENGTH 1024

extern char *lines[MAX_LINES];
extern int lineCount;

int LoadTextFromFile(const char *filename);
void FreeLoadedText(void);

#endif

