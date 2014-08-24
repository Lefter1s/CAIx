#ifndef TUTOSTACKDIALOG_H
#define TUTOSTACKDIALOG_H

#include <QDialog>

namespace Ui {
    class tutoStackDialog;
}
class ClientModel;

class tutoStackDialog : public QDialog
{
    Q_OBJECT

public:
    explicit tutoStackDialog(QWidget *parent = 0);
    ~tutoStackDialog();

    void setModel(ClientModel *model);
private:
    Ui::tutoStackDialog *ui;

private slots:
    void on_buttonBox_accepted();
};

#endif // miningTutDIALOG_H
