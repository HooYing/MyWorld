#ifndef CREATEMAP_H
#define CREATEMAP_H

#include <QWidget>
#include "HashMap.h"

namespace Ui {
class CreateMap;
}

class CreateMap : public QWidget
{
    Q_OBJECT

public:
    explicit CreateMap(QWidget *parent = 0);
    ~CreateMap();

private:
    Ui::CreateMap *ui;
    HashMap hashmap;

public:
    void send_return_start_clicked();
    void send_submit_clicked();

signals:
    void create_map_return_start_clicked_signal();
    void create_map_submit_clicked_signal(bool);
};

#endif // CREATEMAP_H
