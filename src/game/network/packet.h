#define LIBSERIAL_INCLUDE "game/network/packets.h"
#include <libserial.h>
#include <lunarengine.h>

LibSerialObj_Connect* packet_connect();
LibSerialObj_Disconnect* packet_disconnect();
LibSerialObj_SwitchLevel* packet_switch_level();
LibSerialObj_UpdateEntity* packet_entity(LE_Entity* entity);