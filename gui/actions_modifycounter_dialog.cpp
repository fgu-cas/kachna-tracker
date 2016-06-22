#include "actions_modifycounter_dialog.h"
#include "ui_actions_modifycounter_dialog.h"

ActionModifyCounterDialog::ActionModifyCounterDialog(QStringListModel &counters, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ActionModifyCounterDialog)
{
    ui->setupUi(this);
    ui->targetComboBox->setModel(&counters);
}

ActionModifyCounterDialog::~ActionModifyCounterDialog()
{
    delete ui;
}
