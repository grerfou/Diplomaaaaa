#include "symbols.h"
#include "renderer.h"
#include <math.h>

// Exécution temporelle des impressions pour le symbole 𐎠
void UpdateSymbolA(Calame *calame, float timer) {
    calame->pos = (Vector2){400, 300};

    if (timer > 1 && timer < 1.5f) {
        calame->angle = -PI / 2;
        Impression(calame->pos, calame->angle);
    }
    if (timer > 2 && timer < 2.5f) {
        calame->angle = -PI / 4;
        Impression(calame->pos, calame->angle);
    }
    if (timer > 3 && timer < 3.5f) {
        calame->angle = -3 * PI / 4;
        Impression(calame->pos, calame->angle);
    }
}

