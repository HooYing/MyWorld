#ifndef ROOMCLIENT_H
#define ROOMCLIENT_H

#include <thread>
#include <winsock2.h>
#include <windows.h>
#include <mstcpip.h>
#include <mutex>

class RoomClient
{
public:
    static RoomClient* get_instance();
    void roomclient_connect(char* hostname, int port);
    void roomclient_stop();
    void roomclient_send(char* data);
    int roomclient_recv(char* data, int length);
    int roomclient_sendall(int sd, char* data, int length);

    void roomclient_position(int pid, float x, float y, float z, float rx, float ry);
    void roomclient_block(int p, int q, int x, int y, int z, int w);
    void roomclient_room(char* room_name, char* user_name,
         float x, float y, float z, float rx, float ry);
    void roomclient_talk(int pid, char* text);
private:
    static bool enable;
    static const int QUEUE_SIZE = 65536;
    static const int BUFFER_SIZE = 4096;
    std::thread* recv_thread;
    static void* recv_worker();
    static SOCKET sd;
    static char recv_buffer[QUEUE_SIZE];
    static std::mutex mtx;
};

#endif // ROOMCLIENT_H
