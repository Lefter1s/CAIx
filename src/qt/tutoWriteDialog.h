#ifndef TUTOWRITEDIALOG_H
#define TUTOWRITEDIALOG_H

#include <QDialog>

namespace Ui {
    class tutoWriteDialog;
}
class ClientModel;

class tutoWriteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit tutoWriteDialog(QWidget *parent = 0);
    ~tutoWriteDialog();

    void setModel(ClientModel *model);
private:
    Ui::tutoWriteDialog *ui;

private slots:
    void on_buttonBox_accepted();
};

#endif // miningTutDIALOG_H
