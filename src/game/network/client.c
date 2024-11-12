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
#include "main.h"
#include "io/assets/assets.h"

int client_sock;
pthread_t client_thread_id;

void* client_thread(void* data) {
    PacketCallback callback = data;
    while (connection_established(0)) {
        int len;
        read(client_sock, &len, 4);
        unsigned char packet[len];
        read(client_sock, packet, len);
        if (packet[0] == 1) break; // hardcoded disconnect packet handler
        struct Binary binary;
        binary.ptr = packet;
        binary.length = len;
        callback(client_sock, &binary);
    }
    return NULL;
}

void client_connect(const char* hostname, PacketCallback callback) {
    struct sockaddr_in address;
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, hostname, &address.sin_addr) < 0) {
        printf("invalid hostname\n");
        return;
    }
    printf("connecting\n");
    if (connect(client_sock, (struct sockaddr*)&address, sizeof(struct sockaddr_in)) < 0) {
        printf("connection failed: (\n");
        return;
    }
    printf("connected\n");
    open_network(false);
    set_client_socket(client_sock);
    pthread_create(&client_thread_id, NULL, client_thread, callback);
    send_packet(packet_connect());
}

void client_shutdown() {
    send_packet(packet_disconnect(client_player_id));
    pthread_join(client_thread_id, NULL);
}