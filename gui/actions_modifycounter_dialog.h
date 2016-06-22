#ifndef ACTION_MODIFY_COUNTER_DIALOG_H
#define ACTION_MODIFY_COUNTER_DIALOG_H

#include <QDialog>
#include <QStringListModel>

namespace Ui {
class ActionModifyCounterDialog;
}

class ActionModifyCounterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ActionModifyCounterDialog(QStringListModel &counters, QWidget *parent = 0);
    ~ActionModifyCounterDialog();

private:
    Ui::ActionModifyCounterDialog *ui;
};

#endif // ACTION_MODIFY_COUNTER_DIALOG_H
