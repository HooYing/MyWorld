#ifndef GAMEMENU_H
#define GAMEMENU_H

#include <QWidget>

namespace Ui {
class GameMenu;
}

class GameMenu : public QWidget
{
    Q_OBJECT

public:
    explicit GameMenu(QWidget *parent = 0);
    ~GameMenu();

private:
    Ui::GameMenu *ui;

public:
    void send_return_start_signal();
    void send_continue_game_signal();

signals:
    void gamemenu_return_start_clicked_signal();
    void gamemenu_continue_game_clicked_signal();
};

#endif // GAMEMENU_H
