#include "font/font.h"
#include "functions.h"

#include <lunarengine.h>

#include "main.h"
#include "math_util.h"
#include "rng.h"

entity_update(level_finish) {
    float timer = get(entity, "timer", Float, 0);
    timer += delta_time;
    set(entity, "timer", Float, timer);
}

entity_texture(level_finish) {
    float timer = get(entity, "timer", Float, 0);
    float shake = max(0, (30 - timer) / 30) * 16;
    float text_w, text_h;
    const char* text = "${^200}LEVEL CLEAR";
    text_size(&text_w, &text_h, text);
    render_text(drawlist, (WIDTH - text_w) / 2 + random_range(-shake, shake), 64 + random_range(-shake, shake), text);
    return NULL;
}
