#include "CreateRoom.h"
#include "ui_CreateRoom.h"
#include <QDebug>
#include "Client.h"
#include "RoomClient.h"
#include "LoadingMap.h"

CreateRoom::CreateRoom(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreateRoom)
{
    ui->setupUi(this);

    connect(ui->return_start, &QPushButton::clicked, this, &CreateRoom::send_return_start_clicked_signal);
    connect(ui->submit, &QPushButton::clicked, this, &CreateRoom::send_submit_clicked_signal);
}

CreateRoom::~CreateRoom()
{
    delete ui;
}

void CreateRoom::load_map()
{
    ui->select_map->clear();
    Client::get_instance()->client_load_map();
    char buf[1024];
    char name[1024];
    //等待一个切确的答复
    int num;
    while(true){
        int flag = Client::get_instance()->client_recv(buf, 1024);
        if(flag){
            sscanf(buf, "load_map num %d", &num);
            break;
        }
    }
    if(num){
        int cnt = 0;
        while(true){
            int flag = Client::get_instance()->client_recv(buf, 1024);
            if(flag){
                sscanf(buf, "load_map %s", name);
                ui->select_map->addItem(name);
            }
            else{
                break;
            }
        }
    }
}

void CreateRoom::send_return_start_clicked_signal()
{
    emit create_room_return_start_clicked_signal();
}

void CreateRoom::send_submit_clicked_signal()
{
    if(ui->room_name->text() == ""){
        return;
    }
    std::string room_name = ui->room_name->text().toStdString();
    std::string room_map = ui->select_map->currentText().toStdString();
    Client::get_instance()->client_create_room(
                const_cast<char*>(room_name.c_str()), const_cast<char*>(room_map.c_str()));

    char buf[1024];
    char hosthome[] = "120.76.47.97";
    char new_room_name[100];
    char user_name[100];
    int port;
    while(true){
        int flag = Client::get_instance()->client_recv(buf, 1024);
        if(flag){
            sscanf(buf, "%d %s %s", &port, new_room_name, user_name);
            RoomClient::get_instance()->roomclient_connect(hosthome, port);
            break;
        }
    }

    map< pair<int,int>, Map >::iterator it;
    for(it = loadingmap.begin(); it != loadingmap.end(); ++it){
        hashmap.map_free(&(it->second));
    }
    loadingmap.clear();
    is_loadingmap = true;

    while(true){
        int flag = Client::get_instance()->client_recv(buf, 1024);
        if(flag){
            sscanf(buf, "position %f %f %f %f %f",
                   &pos_x, &pos_y, &pos_z, &pos_rx, &pos_ry);

            RoomClient::get_instance()->roomclient_room(new_room_name, user_name,
                                        pos_x, pos_y, pos_z, pos_rx, pos_ry);

            break;
        }
    }

    int num;
    while(true){
        int flag = Client::get_instance()->client_recv(buf, 1024);
        if(flag){
            sscanf(buf, "block num %d\n", &num);
            break;
        }
    }

    int p, q, x, y, z, w;
    while(num){
        int flag = Client::get_instance()->client_recv(buf, 1024);
        if(flag){
            sscanf(buf, "block %d %d %d %d %d %d",
                   &p, &q, &x, &y, &z, &w);
            map< pair<int,int>, Map >::iterator it = loadingmap.find(make_pair(p, q));
            if(it == loadingmap.end()){
                Map new_map;
                hashmap.map_alloc(&new_map);
                hashmap.map_set(&new_map, x, y, z, w);
                loadingmap.insert(make_pair( make_pair(p, q), new_map ));
            }
            else{
                hashmap.map_set(&(it->second), x, y, z, w);
            }

            num--;

        }
    }

    emit create_room_submit_clicked_signal();
}
