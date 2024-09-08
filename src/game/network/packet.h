#define LIBSERIAL_INCLUDE "game/network/packets.h"
#include <libserial.h>
#include <lunarengine.h>

LibSerialObj_Connect* packet_connect();
LibSerialObj_Disconnect* packet_disconnect(int id);
LibSerialObj_PlayerID* packet_player_id(int id);
LibSerialObj_Input* packet_input(int id);
LibSerialObj_RenderedScreen* packet_rendered_screen(LE_DrawList* drawlist);