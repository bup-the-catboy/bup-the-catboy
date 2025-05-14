#include "functions.h"
#include "context.h"
#include "io/io.h"

#include <lunarengine.h>
#include <string.h>

static void shader_controller(void* context, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    char* shader = context_get_ptr(context, "shader");
    bool redraw = context_get_int(context, "redraw");
    float timer = context_get_float(context, "timer");
    context_destroy(context);
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
    return gfxcmd_custom(shader_controller, context_create(
        context_ptr("shader", get(entity, "shader", Ptr, "")),
        context_int("redraw", get(entity, "redraw", Bool, true)),
        context_float("timer", get(entity, "timer", Float, true))
    ));
}
