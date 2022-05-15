#include "Start.h"
#include "ui_Start.h"

Start::Start(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Start)
{
    ui->setupUi(this);

    connect(ui->create_map, &QPushButton::clicked, this, &Start::send_create_map_clicked_signal);
    connect(ui->load_map, &QPushButton::clicked, this, &Start::send_load_map_clicked_signal);
    connect(ui->create_room, &QPushButton::clicked, this, &Start::send_create_room_clicked_signal);
    connect(ui->enter_room, &QPushButton::clicked, this, &Start::send_enter_room_clicked_signal);
    connect(ui->log_out, &QPushButton::clicked, this, &Start::send_log_out_clicked_signal);
}

Start::~Start()
{
    delete ui;
}

void Start::send_create_map_clicked_signal()
{
    emit create_map_clicked_signal();
}

void Start::send_load_map_clicked_signal()
{
    emit load_map_clicked_signal();
}

void Start::send_create_room_clicked_signal()
{
    emit create_room_clicked_signal();
}

void Start::send_enter_room_clicked_signal()
{
    emit enter_room_clicked_signal();
}

void Start::send_log_out_clicked_signal()
{
    emit log_out_clicked_signal();
}


