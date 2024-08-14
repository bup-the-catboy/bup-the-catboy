#include "common.h"

#include "server.h"
#include "client.h"
#include "packet.h"

#include "game/level.h"

#include <sys/socket.h>
#include <unistd.h>

struct PendingPacket {
    unsigned char* data;
    struct PendingPacket* next;
};

int curr_socket = -1;
bool is_server = false;
bool is_open = false;
struct PendingPacket* pending_packets = NULL;
bool processing_packets = false;

void set_socket(int socket, bool server) {
    curr_socket = socket;
    is_server = server;
    is_open = true;
}

void _send_packet(void* packet, bool blocking) {
    if (curr_socket == -1) {
        libserial_objfree(packet);
        return;
    }
    printf("sending packet\n");
    size_t size;
    unsigned char* data = libserial_serialize(packet, &size);
    unsigned char* packet_data = malloc(size + 4);
    *(uint32_t*)packet_data = size;
    memcpy(packet_data + 4, data, size);
    send(curr_socket, packet_data, size + 4, blocking ? 0 : SOCK_NONBLOCK);
    free(packet_data);
    free(data);
    libserial_objfree(packet);
}

void send_packet(void* packet) {
    _send_packet(packet, false);
}

void send_packet_blocking(void* packet) {
    _send_packet(packet, true);
}

void receive_packet(void* packet) {
    switch (libserial_objtype(packet)) {
        case LibSerial_ObjType_Connect: {
            printf("connect received\n");
            send_packet_blocking(packet_switch_level());
        } break;
        case LibSerial_ObjType_Disconnect: {
            printf("disconnect received\n");
            is_open = false;
            close(curr_socket);
        } break;
        case LibSerial_ObjType_SwitchLevel: {
            printf("switch level received\n");
            LibSerialObj_SwitchLevel* level = packet;
            load_level_impl(*level, libserial_arrsize(*level));
        } break;
        case LibSerial_ObjType_UpdateEntity: {
            printf("update entity received\n");
            if (current_level == NULL) break;
            LibSerialObj_UpdateEntity* entity = packet;
            entity->entity_id = entity->entity_id == 0 ? 1 : entity->entity_id;
            printf("deserialized entity_id = %d\n", entity->entity_id);
            LE_EntityList* list = LE_LayerGetDataPointer(LE_LayerGetByIndex(current_level->layers, entity->layer_index));
            LE_EntityListIter* iter = LE_EntityListGetIter(list);
            LE_EntityProperty prop;
            while (iter) {
                LE_Entity* e = LE_EntityListGet(iter);
                if (!LE_EntityGetProperty(e, &prop, "unique_id")) {
                    iter = LE_EntityListNext(iter);
                    continue;
                }
                if (prop.asInt == entity->entity_id) {
                    e->posX = entity->pos_x;
                    e->posY = entity->pos_y;
                    e->velX = entity->vel_x;
                    e->velY = entity->vel_y;
                    e->flags = entity->flags;
                    e->width = entity->width;
                    e->height = entity->height;
                    for (int i = 0; i < libserial_arrsize(entity->properties); i++) {
                        if (strcmp(entity->properties[i].name, "unique_id") == 0) continue;
                        LE_EntitySetProperty(e, (LE_EntityProperty){ .asInt = entity->properties[i].value }, entity->properties[i].name);
                    }
                }
                iter = LE_EntityListNext(iter);
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
    processing_packets = true;
    struct PendingPacket* curr = pending_packets;
    while (curr->next) {
        struct PendingPacket* next = curr->next;
        if (is_open) receive_packet(libserial_deserialize(curr->data));
        free(curr->data);
        free(curr);
        curr = next;
    }
    free(curr);
    pending_packets = NULL;
    init_packet_queue();
    processing_packets = false;
}

void receive_packet_callback(struct Binary* binary) {
    while (processing_packets) usleep(100);
    unsigned char* data = malloc(binary->length);
    memcpy(data, binary->ptr, binary->length);
    struct PendingPacket* curr = pending_packets;
    while (curr->next) curr = curr->next;
    curr->data = data;
    curr->next = malloc(sizeof(struct PendingPacket));
    curr->next->data = NULL;
    curr->next->next = NULL;
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
    if (curr_socket == -1) return;
    is_open = false;
    if (is_server) server_shutdown();
    else client_shutdown();
    close(curr_socket);
}

bool is_socket_open() {
    return is_open;
}
