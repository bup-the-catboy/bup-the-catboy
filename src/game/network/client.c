#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "common.h"
#include "packet.h"

int client;
pthread_t client_thread_id;

void* client_thread(void* data) {
    PacketCallback callback = data;
    while (is_socket_open()) {
        int len;
        read(client, &len, 4);
        unsigned char packet[len];
        read(client, packet, len);
        struct Binary binary;
        binary.ptr = packet;
        binary.length = len;
        callback(&binary);
    }
    return NULL;
}

void client_connect(const char* hostname, PacketCallback callback) {
    struct sockaddr_in address;
    client = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) < 0) {
        printf("invalid hostname\n");
        return;
    }
    printf("connecting\n");
    if (connect(client, (struct sockaddr*)&address, sizeof(struct sockaddr_in)) < 0) {
        printf("connection failed\n");
        return;
    }
    printf("connected\n");
    set_socket(client, false);
    pthread_create(&client_thread_id, NULL, client_thread, callback);
    send_packet(packet_connect());
}

void client_shutdown() {
    send_packet(packet_disconnect());
    pthread_join(client_thread_id, NULL);
}