#ifndef LOGIN_H
#define LOGIN_H

#include <QFrame>
#include <string>
#include <QKeyEvent>
namespace Ui {
class Login;
}

class Login : public QFrame
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

    void keyPressEvent(QKeyEvent *event);

private:
    Ui::Login *ui;

private slots:
    void send_go_register_clicked_signal();

public:
    void on_login_clicked();

signals:
    void go_register_clicked_signal();
    void login_result(bool);
};

#endif // LOGIN_H
