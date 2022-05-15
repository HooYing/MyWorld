#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "Login.h"
#include "Register.h"
#include "Start.h"
#include "CreateMap.h"
#include "LoadMap.h"
#include "World.h"
#include "CreateRoom.h"
#include "EnterRoom.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void world_connect();

private:
    Ui::MainWindow *ui;
    Register *regis;
    Login *login;
    Start *start;
    CreateMap *createmap;
    LoadMap *loadmap;
    World *world;
    CreateRoom *createroom;
    EnterRoom *enterroom;

public:
    void on_go_register_clicked();
    void on_return_login_clicked();
    void deal_login_result(bool);
    void deal_register_result(bool);

    void on_create_map_clicked();
    void on_load_map_clicked();
    void on_create_room_clicked();
    void on_enter_room_clicked();
    void on_log_out_clicked();

    void on_create_map_return_start_clicked();
    void on_create_map_submit_clicked(bool);

    void on_load_map_return_start_clicked();
    void on_loading_map_submit_clicked();

    void on_world_return_start_clicked();

    void on_create_room_return_start_clicked();
    void on_create_room_submit_clicked();

    void on_enter_room_return_start_clicked();
    void on_enter_room_submit_clicked();
};

#endif // MAINWINDOW_H
