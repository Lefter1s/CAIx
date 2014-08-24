#include "tutoStackDialog.h"
#include "ui_tutoStackDialog.h"
#include "clientmodel.h"

#include "version.h"

tutoStackDialog::tutoStackDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::tutoStackDialog)
{
    ui->setupUi(this);
}

void tutoStackDialog::setModel(ClientModel *model)
{
    if(model)
    {
        ui->versionLabel->setText(model->formatFullVersion());
    }
}

tutoStackDialog::~tutoStackDialog()
{
    delete ui;
}

void tutoStackDialog::on_buttonBox_accepted()
{
    close();
}
