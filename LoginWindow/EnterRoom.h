#ifndef ENTERROOM_H
#define ENTERROOM_H

#include <QWidget>
#include <vector>
#include "HashMap.h"
using namespace std;

namespace Ui {
class EnterRoom;
}

class EnterRoom : public QWidget
{
    Q_OBJECT

public:
    explicit EnterRoom(QWidget *parent = 0);
    ~EnterRoom();
    void load_room();

private:
    Ui::EnterRoom *ui;
    vector<int> port_vec;
    vector<string>roommap_vec;
    string username;

    HashMap hashmap;


public:
    void send_return_start_signal();
    void send_submit_signal();

signals:
    void enter_room_return_start_clicked_signal();
    void enter_room_submit_clicked_signal();
};

#endif // ENTERROOM_H
