#include "EnterRoom.h"
#include "ui_EnterRoom.h"
#include "Client.h"
#include <QStandardItemModel>
#include <RoomClient.h>
#include "LoadingMap.h"
#include <QDebug>

EnterRoom::EnterRoom(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EnterRoom)
{
    ui->setupUi(this);

    //ui->tableView->setAlternatingRowColors(true);//设置table隔行变色
    ui->tableView->setFrameShape(QFrame::NoFrame);//设置table无边框
    ui->tableView->setShowGrid(true);//设置table内部线条不显示
    ui->tableView->horizontalHeader()->setStretchLastSection(true);//设置最后一列填满表格
    ui->tableView->resizeColumnsToContents();//设置列宽适应内容调整宽度
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);//设置选中表格时为整行选中
    ui->tableView->setFocusPolicy(Qt::NoFocus);//设置选中表格时没有虚线框（不显示焦点）
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置只读，不可编辑
    //设置table样式，设置表格内容的背景色，字体颜色，大小
    ui->tableView->setStyleSheet("QTableView{color:black;background-color:rgb(173,216,230,0%);\
                                 alternate-background-color:rgb(211,211,211);selection-color:rgb(255,255,255);selection-background-color:rgb(112,128,144);\
                                 border:0px groove gray;padding:2px 4px;border-radius:0px;\
                                 font-family:'Microsoft YaHei';font-size:15px;gridline-color:darkgray;}");

    connect(ui->return_start, &QPushButton::clicked, this, &EnterRoom::send_return_start_signal);
    connect(ui->submit, &QPushButton::clicked, this, &EnterRoom::send_submit_signal);
}

EnterRoom::~EnterRoom()
{
    delete ui;
}

void EnterRoom::load_room()
{
    Client::get_instance()->client_load_room();
    QStandardItemModel *model = new QStandardItemModel();
    model->setColumnCount(1);
    model->setHeaderData(0, Qt::Horizontal, "加入房间");
    char buf[1024];
    char name[1024];
    char user_name[1024];
    char room_map[1024];
    //等待一个切确的答复
    int num;
    while(true){
        int flag = Client::get_instance()->client_recv(buf, 1024);
        if(flag){
            sscanf(buf, "load_room num %d username %s", &num, user_name);
            break;
        }
    }
    username = user_name;
    int cnt = 0;
    while(num){
        int flag = Client::get_instance()->client_recv(buf, 1024);
        if(flag){
            int port;
            sscanf(buf, "load_room %s %s %d", name, room_map, &port);
            port_vec.push_back(port);
            roommap_vec.push_back(room_map);
            model->setItem(cnt, 0, new QStandardItem(name));
            model->item(cnt++, 0)->setTextAlignment(Qt::AlignCenter);
            num--;
        }
    }
    ui->tableView->setModel(model);
}

void EnterRoom::send_return_start_signal()
{
    emit enter_room_return_start_clicked_signal();
}

void EnterRoom::send_submit_signal()
{
    QModelIndex index = ui->tableView->selectionModel()->currentIndex();
    std::string name = index.data(0).toString().toStdString();
    if(name == "")return;
    char hosthome[] = "120.76.47.97";
    RoomClient::get_instance()->roomclient_stop();
    RoomClient::get_instance()->roomclient_connect(hosthome, port_vec[index.row()]);

    string roommap = roommap_vec[index.row()];
    qDebug()<<QString::fromStdString(roommap);
    Client::get_instance()->client_enter_room(const_cast<char*>(roommap.c_str()));


    map< pair<int,int>, Map >::iterator it;
    for(it = loadingmap.begin(); it != loadingmap.end(); ++it){
        hashmap.map_free(&(it->second));
    }
    loadingmap.clear();
    is_loadingmap = true;

    char buf[1024];
    while(true){
        int flag = Client::get_instance()->client_recv(buf, 1024);
        if(flag){
            sscanf(buf, "position %f %f %f %f %f",
                   &pos_x, &pos_y, &pos_z, &pos_rx, &pos_ry);

            RoomClient::get_instance()->roomclient_room(
                        const_cast<char*>(name.c_str()), const_cast<char*>(username.c_str()),
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

    emit enter_room_submit_clicked_signal();
}
