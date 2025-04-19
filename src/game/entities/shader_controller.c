#include "functions.h"
#include "io/io.h"

#include <lunarengine.h>
#include <string.h>

entity_texture(shader_controller) {
    const char* shader = get(entity, "shader", Ptr, "");
    if (strlen(shader) == 0) return graphics_dummy_shader();
    return GET_ASSET(struct GfxResource, shader);
}