#define LIBSERIAL_INCLUDE "game/network/packets.h"
#define LIBSERIAL_SOURCE
#include <libserial.h>
#include <lunarengine.h>

#include "game/level.h"

LibSerialObj_Connect* packet_connect() {
    return libserial_objmake(Connect);
}

LibSerialObj_Disconnect* packet_disconnect() {
    return libserial_objmake(Disconnect);
}

LibSerialObj_SwitchLevel* packet_switch_level() {
    LibSerialObj_SwitchLevel* packet = libserial_objmake(SwitchLevel);
    libserial_arrmake(*packet, current_level->raw_length);
    memcpy(*packet, current_level->raw, current_level->raw_length);
    return packet;
}

LibSerialObj_UpdateEntity* packet_entity(LE_Entity* entity) {
    LibSerialObj_UpdateEntity* packet = libserial_objmake(UpdateEntity);
    LE_EntityProperty prop;
    LE_EntityGetProperty(entity, &prop, "unique_id");
    packet->entity_id = prop.asInt;
    LE_LayerListIter* iter = LE_LayerListGetIter(current_level->layers);
    int index = 0;
    while (iter) {
        if (LE_LayerGetDataPointer(LE_LayerListGet(iter)) == LE_EntityGetList(entity)) {
            packet->layer_index = index;
            break;
        }
        iter = LE_LayerListNext(iter);
        index++;
    }
    packet->pos_x = entity->posX;
    packet->pos_y = entity->posY;
    packet->vel_x = entity->velX;
    packet->vel_y = entity->velY;
    packet->width = entity->width;
    packet->height = entity->height;
    packet->flags = entity->flags;
    int num_props = LE_EntityNumProperties(entity);
    libserial_arrmake(packet->properties, num_props);
    for (int i = 0; i < num_props; i++) {
        const char* key = LE_EntityGetPropertyKey(entity, i);
        strcpy(packet->properties[i].name, key);
        LE_EntityGetProperty(entity, &prop, key);
        packet->properties[i].value = prop.asInt;
    }
    return packet;
}
