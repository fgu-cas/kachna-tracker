#include "actions_modifycounter_dialog.h"
#include "ui_actions_modifycounter_dialog.h"

#include <QCloseEvent>

ActionModifyCounterDialog::ActionModifyCounterDialog(QStringListModel &counters, int row, Action action, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ActionModifyCounterDialog)
{
    ui->setupUi(this);
    ui->targetComboBox->setModel(&counters);

    this->row = row;

    switch (action.type){
    case Action::COUNTER_INC:
        ui->actionComboBox->setCurrentIndex(0);
        break;
    case Action::COUNTER_DEC:
        ui->actionComboBox->setCurrentIndex(1);
        break;
    case Action::COUNTER_SET:
        ui->actionComboBox->setCurrentIndex(2);
        break;
    }

    ui->argSpinBox->setValue(action.arg);
    ui->targetComboBox->setCurrentText(action.target);
}

ActionModifyCounterDialog::~ActionModifyCounterDialog()
{
    delete ui;
}

void ActionModifyCounterDialog::closeEvent(QCloseEvent *event){
    Action action;
    switch (ui->actionComboBox->currentIndex()){
    case 0:
        action.type = Action::COUNTER_INC;
        break;
    case 1:
        action.type = Action::COUNTER_DEC;
        break;
    case 2:
        action.type = Action::COUNTER_SET;
        break;
    }

    action.arg = ui->argSpinBox->value();
    action.target = ui->targetComboBox->currentText();

    emit(update(row, action));
    event->accept();
}
