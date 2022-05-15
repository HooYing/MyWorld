#ifndef CLIENT_H
#define CLIENT_H

#include <thread>
#include <winsock2.h>
#include <windows.h>
#include <mstcpip.h>
#include <mutex>

class Client
{
public:
    static Client* get_instance();
    void client_enable();
    void client_disable();
    int get_client_enabled();
    void client_connect(char* hostname, int port);
    int socket_tcp_alive();
    void client_start();
    void client_stop();
    void client_send(char* data);
    int client_recv(char* data, int length);

    void client_login(char* username, char* passwd);
    void client_register(char* username, char* passwd);

    void client_create_map(char* mapname);
    void client_load_map();
    void client_delete_map(char* mapname);
    void client_loading_map(char* mapname);

    void client_save_block(int p, int q, int x, int y, int z, int w);

    void client_create_room(char* roomname, char* roommap);
    void client_load_room();
    void client_enter_room(char* roommap);
    void client_delete_room();

    void client_position(float x, float y, float z, float rx, float ry);
    void client_chunk(int p, int q, int key);
    void client_block(int x, int y, int z, int w);
    void client_talk(char *text);
    int client_sendall(int sd, char* data, int length);

private:
    static const int QUEUE_SIZE = 65536;
    static const int BUFFER_SIZE = 4096;
    std::thread* recv_thread;
    static void* recv_worker();
    static int client_enabled;
    static SOCKET sd;
    static char recv_buffer[QUEUE_SIZE];
    static std::mutex mtx;
};


#endif // CLIENT_H
