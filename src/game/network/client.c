#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#ifdef WINDOWS
#include <winsock2.h>

int inet_pton(int _, const char* src, void* dst) {
    uint8_t addr[4];
    if (strcmp(src, "localhost") == 0) src = "127.0.0.1";
    char c;
    int num = -1;
    int octet = 0;
    while ((c = *src++) != 0) {
        if (c >= '0' && c <= '9') {
            if (num == -1) num = 0;
            num = num * 10 + (c - '0');
        }
        else if (c == '.') {
            if (num == -1) return 0;
            if (num > 255) return 0;
            if (octet == 4) return 0;
            addr[octet++] = num;
        }
        else return 0;
    }
    if (octet != 4) return 0;
    memcpy(dst, addr, 4);
    return 1;
}

#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

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
    if (inet_pton(AF_INET, hostname, &address.sin_addr) < 0) {
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