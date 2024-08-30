#ifndef BTCB_TRANSITION_H
#define BTCB_TRANSITION_H

#include <lunarengine.h>

#include "math_util.h"

void start_transition(void(*action)(), int length, enum LE_Direction direction, Easing easing);
void render_transition(LE_DrawList* drawlist);
void update_transition();

#endif