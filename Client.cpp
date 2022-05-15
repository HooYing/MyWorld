#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include <iostream>
#include <QDebug>

int Client::client_enabled = 0;
SOCKET Client::sd = 0;
char Client::recv_buffer[QUEUE_SIZE] = {0};
std::mutex Client::mtx;

Client* Client::get_instance(){
    static Client cln;
    return &cln;
}

void Client::client_enable() {
    client_enabled = 1;
}

void Client::client_disable() {
    client_enabled = 0;
}

int Client::get_client_enabled() {
    return client_enabled;
}

int Client::client_sendall(int sd, char *data, int length) {
    if (!client_enabled) {
        return 0;
    }
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

void Client::client_send(char *data) {
    if (!client_enabled) {
        return;
    }
    if (client_sendall(sd, data, strlen(data)) == -1) {
        perror("client_sendall");
        exit(1);
    }
}

void Client::client_login(char* username, char* passwd){
    if(!client_enabled){
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "login %s %s\n", username, passwd);
    client_send(buffer);
}

void Client::client_register(char* username, char* passwd){
    if(!client_enabled){
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "register %s %s\n", username, passwd);
    client_send(buffer);
}

void Client::client_create_map(char *mapname){
    if(!client_enabled){
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "create_map %s\n", mapname);
    client_send(buffer);
}

void Client::client_load_map(){
    if(!client_enabled){
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "load_map\n");
    client_send(buffer);
}

void Client::client_delete_map(char *mapname){
    if(!client_enabled){
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "delete_map %s\n", mapname);
    client_send(buffer);
}

void Client::client_loading_map(char *mapname){
    if(!client_enabled){
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "loading_map %s\n", mapname);
    client_send(buffer);
}

void Client::client_save_block(int p, int q, int x, int y, int z, int w)
{
    if (!client_enabled) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "block %d %d %d %d %d %d\n", p, q, x, y, z, w);
    client_send(buffer);
}

void Client::client_create_room(char *roomname, char *roommap)
{
    if(!client_enabled){
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "create_room %s %s\n", roomname, roommap);
    client_send(buffer);
}

void Client::client_load_room()
{
    if(!client_enabled){
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "load_room\n");
    client_send(buffer);
}

void Client::client_enter_room(char* roommap)
{
    if(!client_enabled){
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "enter_room %s\n", roommap);
    client_send(buffer);
}

void Client::client_delete_room()
{
    if(!client_enabled){
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "delete_room\n");
    client_send(buffer);
}

void Client::client_position(float x, float y, float z, float rx, float ry) {
    if (!client_enabled) {
        return;
    }
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
    snprintf(buffer, 1024, "position %.2f %.2f %.2f %.2f %.2f\n", x, y, z, rx, ry);
    client_send(buffer);
}

void Client::client_chunk(int p, int q, int key) {
    if (!client_enabled) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "C,%d,%d,%d\n", p, q, key);
    client_send(buffer);
}

void Client::client_block(int x, int y, int z, int w) {
    if (!client_enabled) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "B,%d,%d,%d,%d\n", x, y, z, w);
    client_send(buffer);
}

void Client::client_talk(char *text) {
    if (!client_enabled) {
        return;
    }
    if (strlen(text) == 0) {
        return;
    }
    char buffer[1024];
    snprintf(buffer, 1024, "T,%s\n", text);
    client_send(buffer);
}

int Client::client_recv(char *data, int length) {
    if (!client_enabled) {
        return 0;
    }
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

void* Client::recv_worker() {
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

int Client::socket_tcp_alive()
{
    int ret = 0;
    int keep_alive = 1;
    ret = setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, (char*)&keep_alive, sizeof(keep_alive));
    if (ret == SOCKET_ERROR)
    {
        printf("setsockopt failed: %d \n", WSAGetLastError());
        return -1;
    }
    struct tcp_keepalive in_keep_alive = {0};
    unsigned long ul_in_len = sizeof(struct tcp_keepalive);
    struct tcp_keepalive out_keep_alive = {0};
    unsigned long ul_out_len = sizeof(struct tcp_keepalive);
    unsigned long ul_bytes_return = 0;

    in_keep_alive.onoff = 1; /*打开keepalive*/
    in_keep_alive.keepaliveinterval = 5000; /*发送keepalive心跳时间间隔-单位为毫秒*/
    in_keep_alive.keepalivetime = 1000; /*多长时间没有报文开始发送keepalive心跳包-单位为毫秒*/
    ret = WSAIoctl(sd, SIO_KEEPALIVE_VALS, (LPVOID)&in_keep_alive, ul_in_len,
        (LPVOID)&out_keep_alive, ul_out_len, &ul_bytes_return, NULL, NULL);

    if (ret == SOCKET_ERROR)
    {
        printf("WSAIoctl failed: %d \n", WSAGetLastError());
        return -1;
    }
    return 0;
}

void Client::client_connect(char *hostname, int port) {
    if (!client_enabled) {
        return;
    }

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

    if( socket_tcp_alive() < 0){
        perror("socket_tcp_alive");
        exit(1);
    }
}

void Client::client_start() {
    if (!client_enabled) {
        return;
    }

    recv_thread = new std::thread(recv_worker);
}

void Client::client_stop() {
    if (!client_enabled) {
        return;
    }
    client_disable();
    closesocket(sd);
    WSACleanup();

    recv_thread->join();
    delete recv_thread;
}
