#include "functions.h"
#include "io/io.h"

#include <lunarengine.h>
#include <string.h>

static void shader_controller(void* entity, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    char* shader = get(entity, "shader", Ptr, "");
    bool redraw = get(entity, "redraw", Bool, true);
    float timer = get(entity, "timer", Float, 0);
    if (shader[0] != 0) {
        graphics_flush(false);
        graphics_push_shader(shader);
        graphics_shader_set_int("u_level_timer", timer);
        if (redraw) {
            graphics_flush(true);
            graphics_pop_shader();
            return;
        }
    }
    graphics_flush(redraw);
    graphics_pop_shader();
}

entity_texture(shader_controller) {
    return gfxcmd_custom(shader_controller, entity);
}
