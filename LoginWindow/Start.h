#ifndef START_H
#define START_H

#include <QFrame>

namespace Ui {
class Start;
}

class Start : public QFrame
{
    Q_OBJECT

public:
    explicit Start(QWidget *parent = 0);
    ~Start();

private:
    Ui::Start *ui;

public:
    void send_create_map_clicked_signal();
    void send_load_map_clicked_signal();
    void send_create_room_clicked_signal();
    void send_enter_room_clicked_signal();
    void send_log_out_clicked_signal();

signals:
    void create_map_clicked_signal();
    void load_map_clicked_signal();
    void create_room_clicked_signal();
    void enter_room_clicked_signal();
    void log_out_clicked_signal();
};

#endif // START_H
