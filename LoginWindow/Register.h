#ifndef REGISTER_H
#define REGISTER_H

#include <QFrame>

namespace Ui {
class Register;
}

class Register : public QFrame
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = 0);
    ~Register();

private:
    Ui::Register *ui;

private slots:
    void send_return_login_clicked_signal();

public:
    void on_register_clicked();

signals:
    void return_login_clicked_signal();
    void register_result(bool);

};

#endif // REGISTER_H
