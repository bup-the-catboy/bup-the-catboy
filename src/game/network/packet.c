#define LIBSERIAL_INCLUDE "game/network/packets.h"
#define LIBSERIAL_SOURCE
#include <libserial.h>
#include <lunarengine.h>

#include "game/input.h"
#include "game/level.h"

LibSerialObj_Connect* packet_connect() {
    return libserial_objmake(Connect);
}

LibSerialObj_Disconnect* packet_disconnect(int id) {
    LibSerialObj_Disconnect* packet = libserial_objmake(Disconnect);
    *packet = id;
    return packet;
}

LibSerialObj_PlayerID* packet_player_id(int id) {
    LibSerialObj_PlayerID* packet = libserial_objmake(PlayerID);
    *packet = id;
    return packet;
}

LibSerialObj_Input* packet_input(int id) {
    LibSerialObj_Input* packet = libserial_objmake(Input);
    packet->player = id;
    packet->input = 0;
    for (int i = 0; i < NUM_INPUTS; i++) {
        if (is_button_down(id, (1 << i))) packet->input |= (1 << i);
    }
    return packet;
}

LibSerialObj_RenderedScreen* _packet;
int _packet_iter;
void rendered_screen_writer(void* ptr, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    (*_packet)[_packet_iter].dstx  = dstx;
    (*_packet)[_packet_iter].dsty  = dsty;
    (*_packet)[_packet_iter].dstw  = dstw;
    (*_packet)[_packet_iter].dsth  = dsth;
    (*_packet)[_packet_iter].srcx  = srcx;
    (*_packet)[_packet_iter].srcy  = srcy;
    (*_packet)[_packet_iter].srcw  = srcw;
    (*_packet)[_packet_iter].srch  = srch;
    (*_packet)[_packet_iter].color = color;
    if (ptr == 0) memset((*_packet)[_packet_iter++].texture, 0, LIBSERIAL_STRMAXLEN);
    else strcpy((*_packet)[_packet_iter++].texture, get_asset_name(ptr));
}

LibSerialObj_RenderedScreen* packet_rendered_screen(LE_DrawList* dl) {
    LibSerialObj_RenderedScreen* packet = libserial_objmake(RenderedScreen);
    libserial_arrmake(*packet, LE_DrawListSize(dl));
    _packet = packet;
    _packet_iter = 0;
    LE_Render(dl, rendered_screen_writer);
    return packet;
}
