#ifndef BTCB_NETWORK_COMMON_H
#define BTCB_NETWORK_COMMON_H

#include "assets/assets.h"

#include <stdbool.h>

#define PORT 42069 // funny

typedef void(*PacketCallback)(struct Binary*);

void set_socket(int socket, bool server);
void send_packet(void* packet);
void send_packet_blocking(void* packet);
void receive_packet(void* packet);
void process_packets();
void start_server();
void start_client(const char* hostname);
void disconnect();
bool is_socket_open();

#endif