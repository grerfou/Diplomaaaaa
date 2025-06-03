#ifndef MACHINE_H
#define MACHINE_H

#include <stdbool.h>
#include <raylib.h>

bool Machine_Init(const char *filename);
void Machine_Draw(void);
void Machine_Close(void);

#endif // MACHINE_H

