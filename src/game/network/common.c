#include "common.h"

#include "game/input.h"
#include "server.h"
#include "client.h"
#include "packet.h"
#include "main.h"
#include "io/assets/assets.h"

#include "game/level.h"

#ifdef WINDOWS
#include <winsock2.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#endif

#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK 0
#endif

#include <unistd.h>

struct PendingPacket {
    unsigned char* data;
    struct PendingPacket* next;
    int origin;
};

int sockets[MAX_PLAYERS];
bool is_server = false;
bool is_open = false;
struct PendingPacket* pending_packets = NULL;
bool processing_packets = false;

void open_network(bool server) {
    is_server = server;
    is_open = true;
}

void set_client_socket(int socket) {
    sockets[0] = socket;
}

void _send_packet(void* packet, int socket, bool blocking) {
    size_t size;
    unsigned char* data = libserial_serialize(packet, &size);
    unsigned char* packet_data = malloc(size + 4);
    *(uint32_t*)packet_data = size;
    memcpy(packet_data + 4, data, size);
#if defined(WINDOWS)
    u_long mode = !blocking;
    ioctlsocket(socket, FIONBIO, &mode);
#elif defined(MACOS)
    int flags = fcntl(socket, F_GETFL, 0);
    if (blocking) flags &= ~O_NONBLOCK;
    else flags |= O_NONBLOCK;
    fcntl(socket, F_SETFL, flags);
#endif
    printf("sending to %d\n", socket);
    send(socket, packet_data, size + 4, blocking ? 0 : SOCK_NONBLOCK);
    free(packet_data);
    free(data);
    libserial_objfree(packet);
}

void send_packet(void* packet) {
    _send_packet(packet, sockets[0], false);
}

void send_packet_blocking(void* packet) {
    _send_packet(packet, sockets[0], true);
}

void send_packet_to(int connection, void* packet) {
    _send_packet(packet, sockets[connection], false);
}

void send_packet_to_blocking(int connection, void* packet) {
    _send_packet(packet, sockets[connection], true);
}

void send_packet_to_socket(int socket, void* packet) {
    _send_packet(packet, socket, false);
}

void send_packet_to_socket_blocking(int socket, void* packet) {
    _send_packet(packet, socket, true);
}

bool connection_established(int connection) {
    return !!sockets[connection];
}

void receive_packet(int origin, void* packet) {
    switch (libserial_objtype(packet)) {
        case LibSerial_ObjType_Connect: {
            printf("received connect\n");
            int player = create_player(0);
            if (player == -1) send_packet_to_socket(origin, packet_disconnect(0));
            else {
                sockets[player] = origin;
                send_packet_to(player, packet_player_id(player));
            }
        } break;
        case LibSerial_ObjType_Disconnect: {
            printf("received disconnect\n");
            LibSerialObj_Disconnect* pkt = packet;
            if (is_server) {
                printf("client disconnected\n");
                send_packet_to_socket(sockets[*pkt], packet_disconnect(0));
                free(players[*pkt].camera);
                LE_DeleteEntity(players[*pkt].entity);
                players[*pkt].camera = NULL;
                players[*pkt].entity = NULL;
                sockets[*pkt] = 0;
            }
            else {
                close(sockets[0]);
                sockets[0] = 0;
            }
        } break;
        case LibSerial_ObjType_PlayerID: {
            printf("received playerid\n");
            client_player_id = *(LibSerialObj_PlayerID*)packet;
        } break;
        case LibSerial_ObjType_Input: {
            printf("received input\n");
            get_input_from_packet(packet);
        } break;
        case LibSerial_ObjType_RenderedScreen: {
            printf("received renderedscreen\n");
            if (client_drawlist) break;
            LibSerialObj_RenderedScreen* dl = packet;
            client_drawlist = LE_CreateDrawList();
            for (int i = 0; i < libserial_arrsize(*dl); i++) {
                struct Texture* tex = NULL;
                if ((*dl)[i].texture[0]) tex = GET_ASSET(struct Texture, (*dl)[i].texture);
                LE_DrawSetColor(client_drawlist, (*dl)[i].color);
                LE_DrawListAppend(client_drawlist, tex,
                    (*dl)[i].dstx, (*dl)[i].dsty, (*dl)[i].dstw, (*dl)[i].dsth,
                    (*dl)[i].srcx, (*dl)[i].srcy, (*dl)[i].srcw, (*dl)[i].srch
                );
            }
        } break;
        default: break;
    }
    libserial_objfree(packet);
}

void init_packet_queue() {
    if (pending_packets) return;
    pending_packets = malloc(sizeof(struct PendingPacket));
    pending_packets->data = NULL;
    pending_packets->next = NULL;
}

void process_packets() {
    if (!pending_packets) return;
    processing_packets = true;
    struct PendingPacket* curr = pending_packets;
    while (curr->next) {
        struct PendingPacket* next = curr->next;
        if (!curr->data) {
            curr = next;
            continue;
        }
        if (is_open) receive_packet(curr->origin, libserial_deserialize(curr->data));
        free(curr->data);
        free(curr);
        curr = next;
    }
    free(curr);
    pending_packets = NULL;
    init_packet_queue();
    processing_packets = false;
}

void receive_packet_callback(int origin, struct Binary* binary) {
    while (processing_packets) usleep(100);
    unsigned char* data = malloc(binary->length);
    memcpy(data, binary->ptr, binary->length);
    if (*(uint32_t*)data >= LibSerialNumObjects) {
        free(data);
        return;
    }
    struct PendingPacket* curr = pending_packets;
    while (curr->next) curr = curr->next;
    curr->data = data;
    curr->next = malloc(sizeof(struct PendingPacket));
    curr->next->data = NULL;
    curr->next->next = NULL;
    curr->origin = origin;
}

void start_server() {
    init_packet_queue();
    server_init(receive_packet_callback);
}

void start_client(const char* hostname) {
    init_packet_queue();
    client_connect(hostname, receive_packet_callback);
}

void disconnect() {
    if (!is_open) return;
    is_open = false;
    if (is_server) server_shutdown();
    else client_shutdown();
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (sockets[i]) close(sockets[i]);
        sockets[i] = 0;
    }
}
