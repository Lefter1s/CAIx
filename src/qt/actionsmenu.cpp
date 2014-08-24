#include "actionsmenu.h"
#include "ui_actionsmenu.h"
#include "bitcoingui.h"
#include "optionsmodel.h"
#include "walletmodel.h"

ActionsMenu::ActionsMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActionsMenu)
{
    ui->setupUi(this);
}

ActionsMenu::~ActionsMenu()
{
    delete ui;
}

void ActionsMenu::on_EncryptButton_clicked()
{

}

void ActionsMenu::on_BackupButton_clicked()
{
    gui->backupWallet();
}
