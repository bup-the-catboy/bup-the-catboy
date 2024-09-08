#ifndef BTCB_NETWORK_CLIENT_H
#define BTCB_NETWORK_CLIENT_H

#include "game/network/common.h"

extern int client_id;

void client_connect(const char* hostname, PacketCallback callback);
void client_shutdown();

#endif