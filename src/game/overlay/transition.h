#ifndef BTCB_TRANSITION_H
#define BTCB_TRANSITION_H

#include <lunarengine.h>

#include "math_util.h"

bool is_transition_active();
void start_transition(void(*action)(), int length, LE_Direction direction, Easing easing);
void render_transition(LE_DrawList* drawlist);
void update_transition();

#endif