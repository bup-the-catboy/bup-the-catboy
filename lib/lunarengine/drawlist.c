#include <stdlib.h>

#include "linked_list.h"
#include "lunarengine.h"

typedef struct {
    void* texture;
    float dstX, dstY, dstW, dstH;
    int   srcX, srcY, srcW, srcH;
} LE_DrawListEntry;

typedef DEFINE_LIST(LE_DrawListEntry) _LE_DrawList;

LE_DrawList* LE_CreateDrawList() {
    return LE_LL_Create();
}

void LE_Render(LE_DrawList* dl, DrawListRenderer renderer) {
    _LE_DrawList* drawlist = (_LE_DrawList*)dl;
    while (drawlist->next) {
        drawlist = drawlist->next;
        renderer(drawlist->value->texture,
            drawlist->value->dstX, drawlist->value->dstY,
            drawlist->value->dstW, drawlist->value->dstH,
            drawlist->value->srcX, drawlist->value->srcY,
            drawlist->value->srcW, drawlist->value->srcH
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
    LE_LL_Add(dl, e);
}

void LE_ClearDrawList(LE_DrawList* dl) {
    LE_LL_DeepClear(dl, free);
}

void LE_DestroyDrawList(LE_DrawList* dl) {
    LE_LL_DeepFree(dl, free);
}