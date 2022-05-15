#include "RoomClient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <QDebug>


SOCKET RoomClient::sd = 0;
char RoomClient::recv_buffer[QUEUE_SIZE] = {0};
std::mutex RoomClient::mtx;
bool RoomClient::enable = false;

RoomClient* RoomClient::get_instance(){
    static RoomClient cln;
    return &cln;
}

int RoomClient::roomclient_sendall(int sd, char *data, int length) {
    int count = 0;
    while (count < length) {
        int n = send(sd, data + count, length, 0);
        if (n == -1) {
            return -1;
        }
        count += n;
        length -= n;
    }
    return 0;
}

void RoomClient::roomclient_send(char *data) {
    if (roomclient_sendall(sd, data, strlen(data)) == -1) {
        perror("client_sendall");
        exit(1);
    }
}


void RoomClient::roomclient_position(int pid, float x, float y, float z, float rx, float ry) {
    static float px, py, pz, prx, pry = 0;
    float distance =
        (px - x) * (px - x) +
        (py - y) * (py - y) +
        (pz - z) * (pz - z) +
        (prx - rx) * (prx - rx) +
        (pry - ry) * (pry - ry);
    if (distance < 0.1) {
        return;
    }
    px = x; py = y; pz = z; prx = rx; pry = ry;
    char buffer[1024];
    snprintf(buffer, 1024, "position %d %.2f %.2f %.2f %.2f %.2f\n", pid, x, y, z, rx, ry);
    roomclient_send(buffer);
}

void RoomClient::roomclient_block(int p, int q, int x, int y, int z, int w) {
    char buffer[1024];
    snprintf(buffer, 1024, "block %d %d %d %d %d %d\n", p, q, x, y, z, w);
    roomclient_send(buffer);
}

void RoomClient::roomclient_room(char *room_name, char *user_name,
     float x, float y, float z, float rx, float ry) {
    char buffer[1024];
    snprintf(buffer, 1024, "%s %s %.2f %.2f %.2f %.2f %.2f\n", room_name, user_name, x, y, z, rx, ry);
    roomclient_send(buffer);
}

void RoomClient::roomclient_talk(int pid, char *text) {
    char buffer[1024];
    snprintf(buffer, 1024, "talk %d %s\n", pid, text);
    roomclient_send(buffer);
}

int RoomClient::roomclient_recv(char *data, int length) {
    int result = 0;
    mtx.lock();
    char *p = strstr(recv_buffer, "\n");
    if (p) {
        *p = '\0';
        strncpy(data, recv_buffer, length);
        data[length-1] = '\0';
        memmove(recv_buffer, p + 1, strlen(p + 1) + 1);
        result = 1;
    }
    mtx.unlock();
    return result;
}

void* RoomClient::recv_worker() {
    while (1) {
        char data[BUFFER_SIZE] = {0};
        if (recv(sd, data, BUFFER_SIZE - 1, 0) <= 0) {
            perror("recv");
            break;
        }
        while (1) {
            int done = 0;
            mtx.lock();
            if (strlen(recv_buffer) + strlen(data) < QUEUE_SIZE) {
                strcat(recv_buffer, data);
                done = 1;
            }
            mtx.unlock();
            if (done) {
                break;
            }
            Sleep(0);
        }
    }
}

void RoomClient::roomclient_connect(char *hostname, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup");
        exit(1);
    }

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == INVALID_SOCKET) {
        perror("socket");
        exit(1);
    }

    SOCKADDR_IN address;

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(hostname);
    address.sin_port = htons(port);

    if (connect(sd, (SOCKADDR *)&address, sizeof(address)) == SOCKET_ERROR) {
        perror("connect");
        exit(1);
    }

    recv_thread = new std::thread(recv_worker);

    enable = true;
}

void RoomClient::roomclient_stop() {
    if(enable){
        closesocket(sd);
        WSACleanup();

        recv_thread->join();
        delete recv_thread;
        enable = false;
    }
}
