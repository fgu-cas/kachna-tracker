#include "actionssounddialog.h"
#include "ui_actionssounddialog.h"
#include <QCloseEvent>

ActionsSoundDialog::ActionsSoundDialog(int row, Action action, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ActionsSoundDialog)
{
	ui->setupUi(this);
	this->row = row;

	ui->argSpinBox->setValue(action.arg);
}

ActionsSoundDialog::~ActionsSoundDialog()
{
	delete ui;
}

void ActionsSoundDialog::closeEvent(QCloseEvent *event)
{
	Action action;
	action.type = Action::ARENA;

	action.arg = ui->argSpinBox->value();

	emit(update(row, action));
	event->accept();
}