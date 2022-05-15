#ifndef LOADMAP_H
#define LOADMAP_H

#include <QWidget>
#include "HashMap.h"

namespace Ui {
class LoadMap;
}

class LoadMap : public QWidget
{
    Q_OBJECT

public:
    explicit LoadMap(QWidget *parent = 0);
    ~LoadMap();

    void load_map();

private:
    Ui::LoadMap *ui;
    HashMap hashmap;

public:
    void send_return_start_clicked();
    void on_delete_map_clicked();
    void send_loading_map_clicked();

signals:
    void load_map_return_start_clicked_signal();
    void loading_map_clicked_signal();
};

#endif // LOADMAP_H
