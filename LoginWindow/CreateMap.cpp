#include "CreateMap.h"
#include "ui_CreateMap.h"
#include <QMessageBox>
#include <string>
#include "Client.h"
#include <QDebug>
#include "LoadingMap.h"

CreateMap::CreateMap(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreateMap)
{
    ui->setupUi(this);

    connect(ui->return_start, &QPushButton::clicked, this, &CreateMap::send_return_start_clicked);
    connect(ui->submit, &QPushButton::clicked, this, &CreateMap::send_submit_clicked);
}

CreateMap::~CreateMap()
{
    delete ui;
}

void CreateMap::send_return_start_clicked()
{
    emit create_map_return_start_clicked_signal();
}

void CreateMap::send_submit_clicked()
{
    if(ui->map_name->text() == ""){
        QMessageBox::warning(NULL, "警告", "名字不能为空", QMessageBox::Yes);
        return;
    }
    std::string map_name = ui->map_name->text().toStdString();
    Client::get_instance()->client_create_map(const_cast<char *>(map_name.c_str()));

    char buf[1024];
    while(true){
        int flag = Client::get_instance()->client_recv(buf, 1024);
        if(flag){
            char res[10];
            sscanf(buf, "create_map %s\n", res);
            if(strcasecmp(res, "ok") == 0){
                map< pair<int,int>, Map >::iterator it;
                for(it = loadingmap.begin(); it != loadingmap.end(); ++it){
                    hashmap.map_free(&(it->second));
                }
                loadingmap.clear();
                is_loadingmap = false;
                pos_x = pos_y = pos_z = pos_rx = pos_ry = 0;
                emit create_map_submit_clicked_signal(true);
            }
            else{
                emit create_map_submit_clicked_signal(false);
            }
            break;
        }
    }
}
