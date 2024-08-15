#ifndef BTCB_NETWORK_SERVER_H
#define BTCB_NETWORK_SERVER_H

#include "common.h"

void server_init(PacketCallback callback);
void server_shutdown();

#endif