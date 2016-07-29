#include "actions_enabledisabledialog.h"
#include "ui_actions_enabledisabledialog.h"

#include <QCloseEvent>


ActionEnableDisableDialog::ActionEnableDisableDialog(QStringListModel &targets, int row, Action action, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EnableDisableDialog)
{
    ui->setupUi(this);
    ui->targetComboBox->setModel(&targets);

    this->row = row;

    switch (action.type){
    case Action::ENABLE:
        ui->actionComboBox->setCurrentIndex(0);
        break;
    case Action::DISABLE:
        ui->actionComboBox->setCurrentIndex(1);
        break;
    }
    ui->targetComboBox->setCurrentText(action.target);
}

ActionEnableDisableDialog::~ActionEnableDisableDialog()
{
    delete ui;
}

void ActionEnableDisableDialog::closeEvent(QCloseEvent *event){
    Action action;
    switch (ui->actionComboBox->currentIndex()){
    case 0:
        action.type = Action::ENABLE;
        break;
    case 1:
        action.type = Action::DISABLE;
        break;
    default:
        return;
    }

    action.target = ui->targetComboBox->currentText();

    emit(update(row, action));
    event->accept();
}
