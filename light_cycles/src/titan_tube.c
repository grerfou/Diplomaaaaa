#include "titan_tube.h"
#include <math.h>

TitanTube create_tube(float x) {
    TitanTube tube = { .x = x, .color = BLACK, .cue_count = 0 };
    return tube;
}

void add_cue(TitanTube *tube, float start, float end, Color color, float intensity) {
    add_fade_cue(tube, start, end, color, intensity, INTENSITY_CONSTANT);
}

void add_fade_cue(TitanTube *tube, float start, float end, Color color, float intensity, IntensityMode mode) {
    if (tube->cue_count >= MAX_CUES) return;
    TubeCue *cue = &tube->cues[tube->cue_count++];
    cue->start_time = start;
    cue->end_time = end;
    cue->color = color;
    cue->intensity = intensity;
    cue->mode = mode;
}

void update_tube(TitanTube *tube, float time_now) {
    tube->color = BLACK;

    for (int i = 0; i < tube->cue_count; i++) {
        TubeCue *cue = &tube->cues[i];

        if (time_now >= cue->start_time && time_now <= cue->end_time) {
            float t = (time_now - cue->start_time) / (cue->end_time - cue->start_time);
            float f = cue->intensity;

            switch (cue->mode) {
                case INTENSITY_CONSTANT: break;
                case INTENSITY_FADE_IN:  f *= t; break;
                case INTENSITY_FADE_OUT: f *= (1.0f - t); break;
                case INTENSITY_PULSE:    f *= 0.5f * (1 + sinf(t * 2 * PI)); break;
            }

            Color c = cue->color;
            c.r = (unsigned char)(c.r * f);
            c.g = (unsigned char)(c.g * f);
            c.b = (unsigned char)(c.b * f);
            tube->color = c;
            return;
        }
    }
}

void draw_tube(TitanTube *tube, int index) {
    int x = 100 + index * 150;
    int y = 100;
    DrawRectangle(x, y, 100, 30, tube->color);
}

// === CYCLES ===

/*
  *
  *   Division par 3 de cycle tolal correspondant au temps moyen, d'ecriture sur les machine 
  *   
  *   Les 3 phase :
  *
    *   1.Temps d'allumage 
    *   2.Temps constant
    *   3.Temps effacement
  *
  *
*/

void Argile(TitanTube *tubes, int count) {
  // Bougie light
  for (int i = 0; i < count; i++) {
    add_fade_cue(&tubes[i], 0.0f, 20.0f, (Color){255, 255, 240, 255} , 1.0f, INTENSITY_FADE_IN);
    add_fade_cue(&tubes[i], 20.0f, 40.0f, (Color){255, 255, 240, 255}, 1.0f, INTENSITY_CONSTANT);
    add_fade_cue(&tubes[i], 40.0f, 60.0f, (Color){255, 255, 240, 255}, 1.0f, INTENSITY_FADE_OUT);
  }
}

void Papier(TitanTube *tubes, int count) {
  // Bougie light 
  for (int i = 0; i < count; i++) {
    add_fade_cue(&tubes[i], 0.0f, 2.2f, (Color){255, 160, 70, 255} , 1.0f, INTENSITY_FADE_IN);
    add_fade_cue(&tubes[i], 2.2f, 4.4f, (Color){255, 160, 70, 255}, 1.0f, INTENSITY_CONSTANT);
    add_fade_cue(&tubes[i], 4.4f, 6.8f, (Color){255, 160, 70, 255}, 1.0f, INTENSITY_FADE_OUT);
  }
}

void Machine_ecrire(TitanTube *tubes, int count) {
  // Lampe a fioul 
  for (int i = 0; i < count; i++) {
    add_fade_cue(&tubes[i], 0.0f, 1.86f, (Color){255, 120, 30, 200} , 1.0f, INTENSITY_FADE_IN);
    add_fade_cue(&tubes[i], 1.86f, 3.72f, (Color){255, 120, 30, 200}, 1.0f, INTENSITY_CONSTANT);
    add_fade_cue(&tubes[i], 3.72f, 5.6f, (Color){255, 120, 30, 200}, 1.0f, INTENSITY_FADE_OUT);
  }
}

void Old_comp(TitanTube *tubes, int count) {
  // Old electric light
  for (int i = 0; i < count; i++) {
    add_fade_cue(&tubes[i], 0.0f, 1.06f, (Color){255, 180, 100, 255} , 1.0f, INTENSITY_FADE_IN);
    add_fade_cue(&tubes[i], 1.06f, 2.12f, (Color){255, 180, 100, 255}, 1.0f, INTENSITY_CONSTANT);
    add_fade_cue(&tubes[i], 2.12f, 3.20f, (Color){255, 180, 100, 255}, 1.0f, INTENSITY_FADE_OUT);
  }
}

void Modern_comp(TitanTube *tubes, int count) {
  // Modfern electric light 
  for (int i = 0; i < count; i++) {
    add_fade_cue(&tubes[i], 0.0f, 0.56f, (Color){255, 200, 130, 255} , 1.0f, INTENSITY_FADE_IN);
    add_fade_cue(&tubes[i], 0.56f, 1.12f, (Color){255, 200, 130, 255}, 1.0f, INTENSITY_CONSTANT);
    add_fade_cue(&tubes[i], 1.12f, 1.7f, (Color){255, 200, 130, 255}, 1.0f, INTENSITY_FADE_OUT);
  }
}

void Beuz(TitanTube *tubes, int count){
  for (int i = 0; i < count; i++) {
    add_fade_cue(&tubes[i], 0.0f, 0.0f, (Color){255, 200, 130, 255} , 0.0f, INTENSITY_CONSTANT);
  }


}

// ====Boucle des cycles====

static TubeCycle cycles[] = {
    { "Beuz", Beuz, 0.0f},  // 1.7s par mot sur un ordinateur moderne 
    { "Argile", Argile,60.0f},  // 60s par mot sur de l'argile 
    { "Papier", Papier, 6.7f},  // 6.7s par mot sur du papier 
    { "Machine à ecrire", Machine_ecrire, 5.6f},  // 5.6s par mot sur une machine à ecrire
    { "Old Computer", Old_comp, 3.2f},  // 3.2s par mot sur un vielle ordinateur 
    { "Modern Computer", Modern_comp, 1.7f}  // 1.7s par mot sur un ordinateur moderne 
};

int get_available_cycles(const TubeCycle **list) {
    *list = cycles;
    return sizeof(cycles) / sizeof(TubeCycle);
}

