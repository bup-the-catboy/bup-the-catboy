#include <libserial_intellisense.h>

OBJECT(Connect, INT8)
OBJECT(Disconnect, INT8)
OBJECT(SwitchLevel, ARRAY(UINT8))
OBJECT(UpdateEntity, STRUCT(
    UINT8 _(layer_index)
    UINT8 _(entity_id)
    FLOAT _(pos_x)
    FLOAT _(pos_y)
    FLOAT _(vel_x)
    FLOAT _(vel_y)
    FLOAT _(width)
    FLOAT _(height)
    UINT32 _(flags)
    ARRAY(STRUCT(
        STRING _(name)
        UINT32 _(value)
    )) _(properties)
))
OBJECT(SpawnEntity, STRUCT(
    UINT16 _(builder_index)
    FLOAT _(x)
    FLOAT _(y)
))
OBJECT(DespawnEntity, STRUCT(
    UINT8 _(layer_index)
    UINT8 _(entity_id)
))
OBJECT(ModifyTile, STRUCT(
    UINT8 _(layer_index)
    UINT8 _(tile)
    UINT32 _(x)
    UINT32 _(y)
))
