#include <stdlib.h>

#include "linked_list.h"
#include "lunarengine.h"

typedef struct {
    void* texture;
    float dstX, dstY, dstW, dstH;
    int   srcX, srcY, srcW, srcH;
    unsigned int color;
} LE_DrawListEntry;

typedef DEFINE_LIST(LE_DrawListEntry) _LE_DrawList;

LE_DrawList* LE_CreateDrawList() {
    _LE_DrawList* dl = LE_LL_Create();
    dl->value = malloc(sizeof(LE_DrawListEntry));
    dl->value->color = 0xFFFFFFFF;
    return (LE_DrawList*)dl;
}

void LE_Render(LE_DrawList* dl, DrawListRenderer renderer) {
    _LE_DrawList* drawlist = (_LE_DrawList*)dl;
    while (drawlist->next) {
        drawlist = drawlist->next;
        renderer(drawlist->value->texture,
            drawlist->value->dstX, drawlist->value->dstY,
            drawlist->value->dstW, drawlist->value->dstH,
            drawlist->value->srcX, drawlist->value->srcY,
            drawlist->value->srcW, drawlist->value->srcH,
            drawlist->value->color
        );
    }
}

void LE_DrawListAppend(LE_DrawList* dl, void* texture, float dstX, float dstY, float dstW, float dstH, int srcX, int srcY, int srcW, int srcH) {
    LE_DrawListEntry* e = malloc(sizeof(LE_DrawListEntry));
    e->texture = texture;
    e->dstX = dstX; e->dstY = dstY;
    e->dstW = dstW; e->dstH = dstH;
    e->srcX = srcX; e->srcY = srcY;
    e->srcW = srcW; e->srcH = srcH;
    e->color = LE_DrawGetColor(dl);
    LE_LL_Add(dl, e);
}

void LE_DrawSetColor(LE_DrawList* dl, unsigned int rgba) {
    ((_LE_DrawList*)dl)->frst->value->color = rgba;
}

void LE_ClearDrawList(LE_DrawList* dl) {
    LE_LL_DeepClear(dl, free);
}

void LE_DestroyDrawList(LE_DrawList* dl) {
    LE_LL_DeepFree(dl, free);
}

unsigned int LE_DrawGetColor(LE_DrawList* dl) {
    return ((_LE_DrawList*)dl)->frst->value->color;
}