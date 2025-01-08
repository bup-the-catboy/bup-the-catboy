#include <stdlib.h>
#include <time.h>

#include "rng.h"

void random_init() {
    srand(time(NULL));
}

int random_int(int min, int max) {
    return rand() % (max - min + 1) + min;
}

float random_range(float min, float max) {
    return random_float() * (max - min) + min;
}

float random_float() {
    return rand() / (float)RAND_MAX;
}