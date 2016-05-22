#include "actions_enabledisabledialog.h"
#include "ui_actions_enabledisabledialog.h"

ActionEnableDisableDialog::ActionEnableDisableDialog(QStringListModel &targets, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EnableDisableDialog)
{
    ui->setupUi(this);
    ui->targetComboBox->setModel(&targets);
}

ActionEnableDisableDialog::~ActionEnableDisableDialog()
{
    delete ui;
}
