#include "io.h"
#include "io/assets/assets.h"

#include <stdlib.h>
#include <string.h>

void gfxcmd_process(void* resource, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    struct GfxCommand* cmd = resource;
    float texX1 = 0, texX2 = 0, texY1 = 0, texY2 = 0;
    float dstX1 = dstx;
    float dstY1 = dsty;
    float dstX2 = dstx + dstw;
    float dstY2 = dsty + dsth;
    if (dstX2 < dstX1) {
        dstX1 -= dstw;
        dstX2 -= dstw;
    }
    if (dstY2 < dstY1) {
        dstY1 -= dsth;
        dstY2 -= dsth;
    }
    if (cmd) {
        switch (cmd->type) {
            case GfxCmdType_SetTexture:
                texX1 = (float)(srcx       ) / cmd->resource.texture.width;
                texY1 = (float)(srcy       ) / cmd->resource.texture.height;
                texX2 = (float)(srcx + srcw) / cmd->resource.texture.width;
                texY2 = (float)(srcy + srch) / cmd->resource.texture.height;
                graphics_select_texture(&cmd->resource);
                // we dont free since this is likely a GfxResource
                break;
            case GfxCmdType_SelectAndDrawShader:
                graphics_select_shader(&cmd->resource);
                graphics_apply_shader();
                // we dont free since this is likely a GfxResource
                return;
            case GfxCmdType_SelectShader:
                graphics_select_shader(&cmd->resource);
                free(cmd);
                return;
            case GfxCmdType_ShaderSetInt:
                graphics_shader_set_int(cmd->shader_uniform.uniform_name, cmd->shader_uniform.int_value);
                free(cmd);
                return;
            case GfxCmdType_ShaderSetFloat:
                graphics_shader_set_float(cmd->shader_uniform.uniform_name, cmd->shader_uniform.float_value);
                free(cmd);
                return;
            case GfxCmdType_DrawShader:
                graphics_apply_shader();
                free(cmd);
                return;
            case GfxCmdType_Custom:
                cmd->custom.callback(cmd->custom.param);
                free(cmd);
                return;
        }
    }
    else graphics_select_texture(NULL);
    graphics_draw(dstX1, dstY1, dstX2, dstY2, texX1, texY1, texX2, texY2, color);
}

struct GfxCommand* gfxcmd_select_shader(struct GfxResource* resource) {
    struct GfxCommand* cmd = calloc(sizeof(struct GfxCommand), 1);
    memcpy(cmd, resource, sizeof(struct GfxResource));
    cmd->type = GfxCmdType_SelectShader;
    return cmd;
}

struct GfxCommand* gfxcmd_shader_set_int(const char* uniform, int value) {
    struct GfxCommand* cmd = calloc(sizeof(struct GfxCommand), 1);
    cmd->type = GfxCmdType_ShaderSetInt;
    cmd->shader_uniform.uniform_name = uniform;
    cmd->shader_uniform.int_value = value;
    return cmd;
}

struct GfxCommand* gfxcmd_shader_set_float(const char* uniform, float value) {
    struct GfxCommand* cmd = calloc(sizeof(struct GfxCommand), 1);
    cmd->type = GfxCmdType_ShaderSetFloat;
    cmd->shader_uniform.uniform_name = uniform;
    cmd->shader_uniform.float_value = value;
    return cmd;
}

struct GfxCommand* gfxcmd_draw_shader() {
    struct GfxCommand* cmd = calloc(sizeof(struct GfxCommand), 1);
    cmd->type = GfxCmdType_DrawShader;
    return cmd;
}

struct GfxCommand* gfxcmd_custom(void(*func)(void*), void* param) {
    struct GfxCommand* cmd = calloc(sizeof(struct GfxCommand), 1);
    cmd->type = GfxCmdType_Custom;
    cmd->custom.callback = func;
    cmd->custom.param = param;
    return cmd;
}