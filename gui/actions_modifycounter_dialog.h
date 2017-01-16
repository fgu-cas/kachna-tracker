#ifndef ACTION_MODIFY_COUNTER_DIALOG_H
#define ACTION_MODIFY_COUNTER_DIALOG_H

#include <QDialog>
#include <QStringListModel>
#include "params.h"

namespace Ui {
	class ActionModifyCounterDialog;
}

class ActionModifyCounterDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ActionModifyCounterDialog(QStringListModel &counters, int row, Action action, QWidget *parent);
	~ActionModifyCounterDialog();
signals:
	void update(int, Action);
private:
	Ui::ActionModifyCounterDialog *ui;
	int row;

	void closeEvent(QCloseEvent*);
};

#endif // ACTION_MODIFY_COUNTER_DIALOG_H
