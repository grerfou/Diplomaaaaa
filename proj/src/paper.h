#ifndef PAPER_H
#define PAPER_H

#include <stdbool.h>
#include <raylib.h>

bool Paper_Init(const char *filename);
void Paper_Draw(void);
void Paper_Close(void);

#endif // PAPER_H

