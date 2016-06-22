#include "actionssounddialog.h"
#include "ui_actionssounddialog.h"

ActionsSoundDialog::ActionsSoundDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ActionsSoundDialog)
{
    ui->setupUi(this);
}

ActionsSoundDialog::~ActionsSoundDialog()
{
    delete ui;
}

void ActionsSoundDialog::on_comboBox_currentIndexChanged(int index)
{
   ui->durationSpinBox->setEnabled((index == 1));
}
