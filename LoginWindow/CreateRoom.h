#ifndef CREATEROOM_H
#define CREATEROOM_H

#include <QWidget>
#include "HashMap.h"

namespace Ui {
class CreateRoom;
}

class CreateRoom : public QWidget
{
    Q_OBJECT

public:
    explicit CreateRoom(QWidget *parent = 0);
    ~CreateRoom();

private:
    Ui::CreateRoom *ui;
    HashMap hashmap;

public:
    void send_return_start_clicked_signal();
    void send_submit_clicked_signal();

    void load_map();

signals:
    void create_room_return_start_clicked_signal();
    void create_room_submit_clicked_signal();
};

#endif // CREATEROOM_H
