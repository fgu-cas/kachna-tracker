#include "actions_arena_dialog.h"
#include "ui_actions_arena_dialog.h"
#include <QCloseEvent>

ActionArenaDialog::ActionArenaDialog(int row, Action action, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ActionArenaDialog)
{
	ui->setupUi(this);
	this->row = row;

	ui->argSpinBox->setValue(action.arg);
}

ActionArenaDialog::~ActionArenaDialog()
{
	delete ui;
}

void ActionArenaDialog::closeEvent(QCloseEvent *event)
{
	Action action;
	action.type = Action::ARENA;

	action.arg = ui->argSpinBox->value();

	emit(update(row, action));
	event->accept();
}