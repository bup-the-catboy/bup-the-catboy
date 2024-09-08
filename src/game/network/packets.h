#include <libserial_intellisense.h>

OBJECT(Connect, INT8)
OBJECT(Disconnect, INT8)
OBJECT(PlayerID, INT8)
OBJECT(Input, STRUCT(
    UINT8 _(player)
    UINT32 _(input)
))
OBJECT(RenderedScreen, ARRAY(STRUCT(
    STRING _(texture)
    FLOAT _(dstx)
    FLOAT _(dsty)
    FLOAT _(dstw)
    FLOAT _(dsth)
    INT32 _(srcx)
    INT32 _(srcy)
    INT32 _(srcw)
    INT32 _(srch)
    UINT32 _(color)
)))