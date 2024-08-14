#include "common.h"
#include "packet.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

int server;
pthread_t server_thread_id;

void* server_thread(void* data) {
    struct sockaddr* address = ((void**)data)[0];
    socklen_t addrlen = sizeof(struct sockaddr_in);
    PacketCallback callback = ((void**)data)[1];
    listen(server, 3);
    printf("waiting for connection (127.0.0.1:%d)\n", PORT);
    int socket = accept(server, address, &addrlen);
    printf("connection opened\n");
    set_socket(socket, true);
    while (is_socket_open()) {
        int len;
        printf("waiting for packet\n");
        read(socket, &len, 4);
        unsigned char packet[len];
        read(socket, packet, len);
        struct Binary binary;
        binary.ptr = packet;
        binary.length = len;
        callback(&binary);
    }
    printf("server closed\n");
    free(data);
    close(server);
    return NULL;
}

void server_init(PacketCallback callback) {
    int opt;
    struct sockaddr_in* address = malloc(sizeof(struct sockaddr_in));
    server = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);
    if (bind(server, (struct sockaddr*)address, sizeof(struct sockaddr_in)) < 0) {
        printf("server failed to open\n");
        return;
    }
    void** ptrs = malloc(sizeof(void*) * 2);
    ptrs[0] = address;
    ptrs[1] = callback;
    pthread_create(&server_thread_id, NULL, server_thread, ptrs);
}

void server_shutdown() {
    send_packet(packet_disconnect());
    pthread_join(server_thread_id, NULL);
}