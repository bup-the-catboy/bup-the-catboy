#ifndef BTCB_NETWORK_COMMON_H
#define BTCB_NETWORK_COMMON_H

#include "io/assets/assets.h"

#include <stdbool.h>

#define PORT 42069 // funny

typedef void(*PacketCallback)(int origin, struct Binary*);

void open_network(bool server);
void set_client_socket(int socket);
void send_packet(void* packet);
void send_packet_blocking(void* packet);
void send_packet_to(int connection, void* packet);
void send_packet_to_blocking(int connection, void* packet);
void send_packet_to_socket(int connection, void* packet);
void send_packet_to_socket_blocking(int connection, void* packet);
bool connection_established(int connection);
void process_packets();
void start_server();
void start_client(const char* hostname);
void disconnect();

#endif