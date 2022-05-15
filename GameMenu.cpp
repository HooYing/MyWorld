#include "GameMenu.h"
#include "ui_GameMenu.h"

GameMenu::GameMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameMenu)
{
    ui->setupUi(this);
    ui->continue_game->setFocusPolicy(Qt::NoFocus);
    ui->return_start->setFocusPolicy(Qt::NoFocus);
    connect(ui->continue_game, &QPushButton::clicked, this, &GameMenu::send_continue_game_signal);
    connect(ui->return_start, &QPushButton::clicked, this, &GameMenu::send_return_start_signal);
}

GameMenu::~GameMenu()
{
    delete ui;
}

void GameMenu::send_continue_game_signal()
{
    emit gamemenu_continue_game_clicked_signal();
}

void GameMenu::send_return_start_signal()
{
    emit gamemenu_return_start_clicked_signal();
}
