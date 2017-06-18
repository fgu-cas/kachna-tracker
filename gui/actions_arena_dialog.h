#ifndef ACTION_ARENA_DIALOG_H
#define ACTION_ARENA_DIALOG_H

#include <QDialog>
#include <QStringListModel>
#include "params.h"

namespace Ui {
	class ActionArenaDialog;
}

class ActionArenaDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ActionArenaDialog(int row, Action action, QWidget *parent);
	~ActionArenaDialog();
signals:
	void update(int, Action);
private:
	Ui::ActionArenaDialog *ui;
	int row;

	void closeEvent(QCloseEvent*);
};

#endif // ACTION_MODIFY_COUNTER_DIALOG_H
