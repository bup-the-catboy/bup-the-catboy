#include "io.h"
#include "io/assets/assets.h"
#include "main.h"
#include "math_util.h"

#include <stdlib.h>
#include <string.h>

void gfxcmd_process(void* resource, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    if (!rect_intersects_rect(0, 0, WIDTH, HEIGHT, dstx, dsty, dstw, dsth)) return;
    struct GfxCommand* cmd = resource;
    cmd->callback(cmd->param, dstx, dsty, dstw, dsth, srcx, srcy, srcw, srch, color);
    if (!cmd->eternal) free(cmd);
}

static void gfxcmd_draw_texture(void* param, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
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
    struct Texture* tex = param;
    if (tex) {
        texX1 = (float)(srcx       ) / tex->width;
        texY1 = (float)(srcy       ) / tex->height;
        texX2 = (float)(srcx + srcw) / tex->width;
        texY2 = (float)(srcy + srch) / tex->height;
    }
    graphics_select_texture(tex);
    graphics_draw(dstX1, dstY1, dstX2, dstY2, texX1, texY1, texX2, texY2, color);
}

struct GfxCommand* gfxcmd_texture(const char* path) {
    struct GfxCommand* cmd = calloc(sizeof(struct GfxCommand), 1);
    cmd->callback = gfxcmd_draw_texture;
    cmd->param = path ? GET_ASSET(struct Texture, path) : NULL;
    return cmd;
}

struct GfxCommand* gfxcmd_custom(GfxCmdCustom func, void* param) {
    struct GfxCommand* cmd = calloc(sizeof(struct GfxCommand), 1);
    cmd->callback = func;
    cmd->param = param;
    return cmd;
}

struct GfxCommand* gfxcmd_eternal(struct GfxCommand* cmd) {
    cmd->eternal = true;
    return cmd;
}