#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include "Client.h"
#include "RoomClient.h"
#include <QMessageBox>
#include "LoadingMap.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("wlecome");
    setWindowIcon(QIcon(":/resource/favicon.ico"));

    login = new Login();
    regis = new Register();
    start = new Start();
    createmap = new CreateMap();
    loadmap = new LoadMap();
    createroom = new CreateRoom();
    enterroom = new EnterRoom();
    world = NULL;
    ui->gridLayout->addWidget(login, 1, 1, 1, 1);

    Client::get_instance()->client_enable();
    char hosthome[] = "120.76.47.97";
    Client::get_instance()->client_connect(hosthome, 9190);
    Client::get_instance()->client_start();


    /* Login */
    void (Login::*go_register_signal)() = &Login::go_register_clicked_signal;
    connect(login, go_register_signal, this, &MainWindow::on_go_register_clicked);
    
    void (Login::*login_signal)(bool) = &Login::login_result;
    connect(login, login_signal, this, &MainWindow::deal_login_result);

    /* Register */
    void (Register::*return_login_signal)() = &Register::return_login_clicked_signal;
    connect(regis, return_login_signal, this, &MainWindow::on_return_login_clicked);

    void (Register::*register_signal)(bool) = &Register::register_result;
    connect(regis, register_signal, this, &MainWindow::deal_register_result);

    /* Start */
    void (Start::*create_map_signal)() = &Start::create_map_clicked_signal;
    connect(start, create_map_signal, this, &MainWindow::on_create_map_clicked);

    void (Start::*load_map_signal)() = &Start::load_map_clicked_signal;
    connect(start, load_map_signal, this, &MainWindow::on_load_map_clicked);

    void (Start::*create_room_signal)() = &Start::create_room_clicked_signal;
    connect(start, create_room_signal, this, &MainWindow::on_create_room_clicked);

    void (Start::*enter_room_signal)() = &Start::enter_room_clicked_signal;
    connect(start, enter_room_signal, this, &MainWindow::on_enter_room_clicked);

    void (Start::*log_out_signal)() = &Start::log_out_clicked_signal;
    connect(start, log_out_signal, this, &MainWindow::on_log_out_clicked);

    /* create_map */
    void (CreateMap::*create_map_return_start_signal)() = &CreateMap::create_map_return_start_clicked_signal;
    connect(createmap, create_map_return_start_signal, this, &MainWindow::on_create_map_return_start_clicked);

    void (CreateMap::*create_map_submit_signal)(bool) = &CreateMap::create_map_submit_clicked_signal;
    connect(createmap, create_map_submit_signal, this, &MainWindow::on_create_map_submit_clicked);

    /* load_map */
    void (LoadMap::*load_map_return_start_signal)() = &LoadMap::load_map_return_start_clicked_signal;
    connect(loadmap, load_map_return_start_signal, this, &MainWindow::on_load_map_return_start_clicked);

    void (LoadMap::*loading_map_submit_signal)() = &LoadMap::loading_map_clicked_signal;
    connect(loadmap, loading_map_submit_signal, this, &MainWindow::on_loading_map_submit_clicked);

    /* create_room */
    void (CreateRoom::*create_room_return_start_signal)() = &CreateRoom::create_room_return_start_clicked_signal;
    connect(createroom, create_room_return_start_signal, this, &MainWindow::on_create_room_return_start_clicked);

    void (CreateRoom::*create_room_submit_signal)() = &CreateRoom::create_room_submit_clicked_signal;
    connect(createroom, create_room_submit_signal, this, &MainWindow::on_create_room_submit_clicked);

    /* enter_room */
    void (EnterRoom::*enter_room_return_start_signal)() = &EnterRoom::enter_room_return_start_clicked_signal;
    connect(enterroom, enter_room_return_start_signal, this, &MainWindow::on_enter_room_return_start_clicked);

    void (EnterRoom::*enter_room_submit_signal)() = &EnterRoom::enter_room_submit_clicked_signal;
    connect(enterroom, enter_room_submit_signal, this, &MainWindow::on_enter_room_submit_clicked);

}

MainWindow::~MainWindow()
{
    delete ui;
    delete login;
    delete regis;
    delete start;
    delete createmap;
}

void MainWindow::world_connect()
{
    void (World::*world_return_start_signal)() = &World::world_return_start_clicked_signal;
    connect(world, world_return_start_signal, this, &MainWindow::on_world_return_start_clicked);
}

void MainWindow::on_go_register_clicked()
{
    login->setParent(nullptr);
    ui->gridLayout->removeWidget(login);
    ui->gridLayout->addWidget(regis, 1, 1, 1, 1);
}

void MainWindow::on_return_login_clicked()
{
    regis->setParent(nullptr);
    ui->gridLayout->removeWidget(regis);
    ui->gridLayout->addWidget(login, 1, 1, 1, 1);
}

void MainWindow::deal_login_result(bool result)
{
    if(result){
        login->setParent(nullptr);
        ui->gridLayout->removeWidget(login);
        ui->gridLayout->addWidget(start, 1, 1, 1, 1);
    }
    else{
        QMessageBox::warning(NULL, "警告", "登录失败", QMessageBox::Yes);
    }
}

void MainWindow::deal_register_result(bool result)
{
    if(result){
        QMessageBox::information(NULL, "信息", "注册成功", QMessageBox::Yes);
    }
    else{
        QMessageBox::warning(NULL, "警告", "用户名存在", QMessageBox::Yes);
    }
}

void MainWindow::on_create_map_clicked()
{
    start->setParent(nullptr);
    ui->gridLayout->removeWidget(start);
    ui->gridLayout->addWidget(createmap, 1, 1, 1, 1);
}

void MainWindow::on_load_map_clicked()
{
    loadmap->load_map();
    start->setParent(nullptr);
    ui->gridLayout->removeWidget(start);
    ui->gridLayout->addWidget(loadmap, 1, 1, 1, 1);
}

void MainWindow::on_create_room_clicked()
{
    createroom->load_map();
    start->setParent(nullptr);
    ui->gridLayout->removeWidget(start);
    ui->gridLayout->addWidget(createroom, 1, 1, 1, 1);
}

void MainWindow::on_enter_room_clicked()
{
    enterroom->load_room();
    start->setParent(nullptr);
    ui->gridLayout->removeWidget(start);
    ui->gridLayout->addWidget(enterroom, 1, 1, 1, 1);
}

void MainWindow::on_log_out_clicked()
{
    start->setParent(nullptr);
    ui->gridLayout->removeWidget(start);
    ui->gridLayout->addWidget(login, 1, 1, 1, 1);
}

void MainWindow::on_create_map_return_start_clicked()
{
    createmap->setParent(nullptr);
    ui->gridLayout->removeWidget(createmap);
    ui->gridLayout->addWidget(start, 1, 1, 1, 1);
}

void MainWindow::on_create_map_submit_clicked(bool result)
{
    if(result){
        if(world!=NULL){
            delete world;
        }
        world = new World();
        world_connect();
        world->show();
        this->hide();
    }
    else{
        QMessageBox::warning(NULL, "警告", "您已使用过该用户名", QMessageBox::Yes);
    }
}

void MainWindow::on_load_map_return_start_clicked()
{
    loadmap->setParent(nullptr);
    ui->gridLayout->removeWidget(loadmap);
    ui->gridLayout->addWidget(start, 1, 1, 1, 1);
}

void MainWindow::on_loading_map_submit_clicked()
{
    if(world!=NULL){
        delete world;
    }
    world = new World();
    world_connect();
    world->show();
    this->hide();
}

void MainWindow::on_world_return_start_clicked()
{
    world->close();
    this->show();
}

void MainWindow::on_create_room_return_start_clicked()
{
    createroom->setParent(nullptr);
    ui->gridLayout->removeWidget(createroom);
    ui->gridLayout->addWidget(start, 1, 1, 1, 1);
}

void MainWindow::on_create_room_submit_clicked()
{
    if(world!=NULL){
        delete world;
    }
    world = new World(0, true);
    world_connect();
    world->show();
    this->hide();
}

void MainWindow::on_enter_room_return_start_clicked()
{
    enterroom->setParent(nullptr);
    ui->gridLayout->removeWidget(enterroom);
    ui->gridLayout->addWidget(start, 1, 1, 1, 1);
}

void MainWindow::on_enter_room_submit_clicked()
{
    if(world!=NULL){
        delete world;
    }
    world = new World(0, true, true);
    world_connect();
    world->show();
    this->hide();
}
