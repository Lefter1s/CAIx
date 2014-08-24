#include "tutoWriteDialog.h"
#include "ui_tutoWriteDialog.h"
#include "clientmodel.h"

#include "version.h"

tutoWriteDialog::tutoWriteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::tutoWriteDialog)
{
    ui->setupUi(this);
}

void tutoWriteDialog::setModel(ClientModel *model)
{
    if(model)
    {
        ui->versionLabel->setText(model->formatFullVersion());
    }
}

tutoWriteDialog::~tutoWriteDialog()
{
    delete ui;
}

void tutoWriteDialog::on_buttonBox_accepted()
{
    close();
}
