#ifndef ACTIONARENADIALOG_H
#define ACTIONARENADIALOG_H

#include <QDialog>
#include "params.h"

namespace Ui {
	class ActionArenaDialog;
}

class ActionArenaDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ActionArenaDialog(int row, Action action, QWidget *parent = 0);
	~ActionArenaDialog();
signals:
	void update(int, Action);
private:
	Ui::ActionArenaDialog *ui;
	int row;

	void closeEvent(QCloseEvent*);
};

#endif // ACTIONARENADIALOG_H