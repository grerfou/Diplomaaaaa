#include "text_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *lines[MAX_LINES] = {0};
int lineCount = 0;

int LoadTextFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
if (!file) return 0;

    lineCount = 0;

    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, MAX_LINE_LENGTH, file) && lineCount < MAX_LINES) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        lines[lineCount] = (char*)malloc(len + 1);
        if (!lines[lineCount]) {
            // Échec malloc : on libère ce qui a été alloué avant
            for (int i = 0; i < lineCount; i++) {
                free(lines[i]);
                lines[i] = NULL;
            }
            lineCount = 0;
            fclose(file);
            return 0;
        }
        strcpy(lines[lineCount], buffer);
        lineCount++;
    }

    fclose(file);
    return 1;
}

void FreeLoadedText(void) {
    for (int i = 0; i < lineCount; i++) {
        if (lines[i]) {
            free(lines[i]);
            lines[i] = NULL;
        }
    }
    lineCount = 0;
}

