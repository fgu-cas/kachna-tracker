#ifndef ENABLEDISABLEDIALOG_H
#define ENABLEDISABLEDIALOG_H

#include <QDialog>
#include <QStringListModel>

namespace Ui {
class EnableDisableDialog;
}

class ActionEnableDisableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ActionEnableDisableDialog(QStringListModel &targets, QWidget *parent = 0);
    ~ActionEnableDisableDialog();

private:
    Ui::EnableDisableDialog *ui;
};

#endif // ENABLEDISABLEDIALOG_H
