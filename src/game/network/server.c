#include "common.h"
#include "packet.h"
#include "main.h"
#include "io/assets/assets.h"

#ifdef WINDOWS
#include <winsock2.h>
#define SO_REUSEPORT 0
typedef int socklen_t;
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>

int server;
pthread_t server_thread_id;
pthread_t server_connection_threads[MAX_PLAYERS];
bool server_connections[MAX_PLAYERS];

#define GET_ARG(type, index) ((type)((void**)data)[index])
void* server_connection_thread(void* data) {
    int index = *GET_ARG(int*, 0);
    int socket = *GET_ARG(int*, 1);
    PacketCallback callback = GET_ARG(PacketCallback, 2);
    server_connections[index] = true;
    printf("connection opened\n");
    while (true) {
        int len;
        if (read(socket, &len, 4) < 0) break;
        unsigned char packet[len];
        read(socket, packet, len);
        struct Binary binary;
        binary.ptr = packet;
        binary.length = len;
        callback(socket, &binary);
    }
    printf("connection closed\n");
    server_connections[index] = false;
    return NULL;
}

void* server_thread(void* data) {
    PacketCallback callback = data;
    listen(server, 3);
    printf("waiting for connection (127.0.0.1:%d)\n", PORT);
    while (server) {
        int socket = accept(server, NULL, NULL);
        if (socket == -1) continue;
        int i = 0;
        for (; i < MAX_PLAYERS; i++) {
            if (!server_connections[i]) break;
        }
        if (i == 16) send_packet_to_socket(socket, packet_disconnect(0));
        else pthread_create(server_connection_threads + i, NULL, server_connection_thread, (void*[]){ &i, &socket, callback });
    }
    printf("server closed\n");
    return NULL;
}

void server_init(PacketCallback callback) {
#ifdef WINDOWS
    char opt;
#else
    int opt;
#endif
    struct sockaddr_in* address = malloc(sizeof(struct sockaddr_in));
    server = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);
    if (bind(server, (struct sockaddr*)address, sizeof(struct sockaddr_in)) < 0) {
        printf("server failed to open: (%d) %s\n", errno, strerror(errno));
        return;
    }
    open_network(true);
    pthread_create(&server_thread_id, NULL, server_thread, callback);
}

void server_shutdown() {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (connection_established(i)) send_packet_to(i, packet_disconnect(0));
    }
    close(server);
    server = 0;
    pthread_cancel(server_thread_id);
}