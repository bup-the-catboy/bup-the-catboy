#include "transition.h"

#include <SDL2/SDL.h>

#include "assets/assets.h"
#include "math_util.h"
#include "main.h"

#define TRANSITION_PADDING 32

struct {
    bool active;
    Easing easing;
    void(*action)();
    int length;
    int frame;
    enum LE_Direction direction;
    float x, y;
} curr_transition;

void start_transition(void(*action)(), int length, enum LE_Direction direction, Easing easing) {
    if (curr_transition.active) return;
    curr_transition.frame = 0;
    curr_transition.length = length;
    curr_transition.easing = easing;
    curr_transition.action = action;
    curr_transition.direction = direction;
    curr_transition.active = true;
    update_transition();
    curr_transition.frame = 0;
}

void render_transition(LE_DrawList* drawlist) {
    if (!curr_transition.active) return;
    int w = TRANSITION_PADDING * 2 + WIDTH;
    int h = TRANSITION_PADDING * 2 + HEIGHT;
    LE_DrawListAppend(drawlist, GET_ASSET(SDL_Texture, "images/transition.png"), curr_transition.x, curr_transition.y, w, h, 0, 0, w, h);
}

void update_transition() {
    if (!curr_transition.active) return;
    if (curr_transition.frame == curr_transition.length) {
        curr_transition.active = false;
        return;
    }
    curr_transition.x = -TRANSITION_PADDING;
    curr_transition.y = -TRANSITION_PADDING;
    if (curr_transition.frame == curr_transition.length / 2 + 1) curr_transition.action();
    switch (curr_transition.direction) {
        case LE_Direction_Up:
            curr_transition.y = lerp(curr_transition.easing(curr_transition.frame / (float)curr_transition.length), -HEIGHT - TRANSITION_PADDING * 2, HEIGHT);
            break;
        case LE_Direction_Left:
            curr_transition.x = lerp(curr_transition.easing(curr_transition.frame / (float)curr_transition.length), -WIDTH - TRANSITION_PADDING * 2, WIDTH);
            break;
        case LE_Direction_Down:
            curr_transition.y = lerp(curr_transition.easing(curr_transition.frame / (float)curr_transition.length), HEIGHT, -HEIGHT - TRANSITION_PADDING * 2);
            break;
        case LE_Direction_Right:
            curr_transition.x = lerp(curr_transition.easing(curr_transition.frame / (float)curr_transition.length), WIDTH, -WIDTH - TRANSITION_PADDING * 2);
            break;
    }
    curr_transition.frame++;
}
