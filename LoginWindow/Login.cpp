#include "Login.h"
#include "ui_Login.h"
#include <QMessageBox>
#include <QDebug>
#include "Client.h"
#include <windows.h>

Login::Login(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    connect(ui->go_register, &QPushButton::clicked, this, &Login::send_go_register_clicked_signal);
    connect(ui->login, &QPushButton::clicked, this, &Login::on_login_clicked);
}

Login::~Login()
{
    delete ui;
}

void Login::send_go_register_clicked_signal()
{
    emit go_register_clicked_signal();
}

void Login::on_login_clicked()
{
    if(ui->username->text()==""){
        QMessageBox::warning(NULL, "警告", "用户名不能为空", QMessageBox::Yes);
        return;
    }
    else if(ui->passwd->text()==""){
        QMessageBox::warning(NULL, "警告", "密码不能为空", QMessageBox::Yes);
        return;
    }

    std::string user_str = ui->username->text().toStdString();
    std::string pass_str = ui->passwd->text().toStdString();
    Client::get_instance()->client_login(
                const_cast<char *>(user_str.c_str()), const_cast<char *>(pass_str.c_str()));

    char buf[1024];
    while(true){
        int flag = Client::get_instance()->client_recv(buf, 1024);
        if(flag){
            char res[10];
            sscanf(buf, "login %s", res);
            if(strcasecmp(res, "ok") == 0){
                emit login_result(true);
            }
            else{
                emit login_result(false);
            }
            break;
        }
    }
}

void Login::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return){
        on_login_clicked();
    }
}


