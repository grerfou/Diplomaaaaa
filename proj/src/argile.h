#ifndef ARGILE_H
#define ARGILE_H

#include <stdbool.h>
#include <raylib.h>

extern int argileMaxLength;

//extern float argileAnglePitch;
//extern float argileAngleYaw;
//extern float argileTextAngle;


//extern int argileTextWidth;
extern Vector2 argileTextPos;
extern int argileFontSize;
extern int argileLineSpacing;
extern float argileFadeSpeed;

bool Argile_Init(const char *filename);
void Argile_Draw(void);
void Argile_Close(void);

#endif

