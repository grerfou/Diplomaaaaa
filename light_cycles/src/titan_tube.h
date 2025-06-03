#ifndef TITAN_TUBE_H
#define TITAN_TUBE_H

#include "raylib.h"

#define MAX_CUES 64

typedef enum {
    INTENSITY_CONSTANT,
    INTENSITY_FADE_IN,
    INTENSITY_FADE_OUT,
    INTENSITY_PULSE
} IntensityMode;

typedef struct {
    float start_time;
    float end_time;
    Color color;
    float intensity;
    IntensityMode mode;
} TubeCue;

typedef struct {
    float x;
    Color color;
    TubeCue cues[MAX_CUES];
    int cue_count;
} TitanTube;

typedef struct {
    const char *name;
    void (*setup)(TitanTube *tubes, int count);
    float duration; // << NOUVEAU
} TubeCycle;


TitanTube create_tube(float x);
void add_cue(TitanTube *tube, float start, float end, Color color, float intensity);
void add_fade_cue(TitanTube *tube, float start, float end, Color color, float intensity, IntensityMode mode);
void update_tube(TitanTube *tube, float time_now);
void draw_tube(TitanTube *tube, int index);
int get_available_cycles(const TubeCycle **cycles);

#endif

