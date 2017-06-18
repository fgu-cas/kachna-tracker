#include "actions_arena_dialog.h"
#include "ui_actions_arena_dialog.h"

#include <QCloseEvent>

ActionArenaDialog::ActionArenaDialog(int row, Action action, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ActionArenaDialog)
{
	ui->setupUi(this);
	/*
	ui->argSpinBox->setValue(action.arg);
	ui->targetComboBox->setCurrentText(action.target);
	*/
}

ActionArenaDialog::~ActionArenaDialog()
{
	delete ui;
}

void ActionArenaDialog::closeEvent(QCloseEvent *event) {
	/*
	Action action;
	switch (ui->actionComboBox->currentIndex()) {
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
	*/
	event->accept();
}
