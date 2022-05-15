#include "Register.h"
#include "ui_Register.h"
#include <QMessageBox>
#include "Client.h"

Register::Register(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
    connect(ui->return_login, &QPushButton::clicked, this, &Register::send_return_login_clicked_signal);
    connect(ui->submit, &QPushButton::clicked, this, &Register::on_register_clicked);
}

Register::~Register()
{
    delete ui;
}

void Register::send_return_login_clicked_signal()
{
    emit return_login_clicked_signal();
}

void Register::on_register_clicked()
{
    if(ui->username->text()==""){
        QMessageBox::warning(NULL, "警告", "用户名不能为空", QMessageBox::Yes);
        return;
    }
    else if(ui->passwd->text()==""){
        QMessageBox::warning(NULL, "警告", "密码不能为空", QMessageBox::Yes);
        return;
    }
    else if(ui->passwd->text()!=ui->passwd_2->text()){
        QMessageBox::warning(NULL, "警告", "密码不一致", QMessageBox::Yes);
        return;
    }

    std::string user_str = ui->username->text().toStdString();
    std::string pass_str = ui->passwd->text().toStdString();
    Client::get_instance()->client_register(
                const_cast<char *>(user_str.c_str()), const_cast<char *>(pass_str.c_str()));

    char buf[1024];
    while(true){
        int flag = Client::get_instance()->client_recv(buf, 1024);
        if(flag){
            char res[10];
            sscanf(buf, "register %s", res);
            if(strcasecmp(res, "ok") == 0){
                emit register_result(true);
            }
            else{
                emit register_result(false);
            }
            break;
        }
    }
}
