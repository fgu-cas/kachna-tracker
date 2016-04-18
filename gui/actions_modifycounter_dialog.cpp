#include "actions_modifycounter_dialog.h"
#include "ui_actions_modifycounter_dialog.h"

ActionModifyCounterDialog::ActionModifyCounterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ActionModifyCounterDialog)
{
    ui->setupUi(this);
}

ActionModifyCounterDialog::~ActionModifyCounterDialog()
{
    delete ui;
}
