#ifndef ACTIONSMENU_H
#define ACTIONSMENU_H

#include <QWidget>

namespace Ui {
    class ActionsMenu;
}

class BitcoinGUI;

class ActionsMenu : public QWidget
{
    Q_OBJECT

public:
    explicit ActionsMenu(QWidget *parent = 0);
    ~ActionsMenu();

private slots:
    void on_EncryptButton_clicked();

    void on_BackupButton_clicked();

private:
    Ui::ActionsMenu *ui;
    BitcoinGUI *gui;
};

#endif // ACTIONSMENU_H
