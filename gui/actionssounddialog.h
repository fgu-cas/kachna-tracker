#ifndef ACTIONSSOUNDDIALOG_H
#define ACTIONSSOUNDDIALOG_H

#include <QDialog>
#include "params.h"

namespace Ui {
	class ActionsSoundDialog;
}

class ActionsSoundDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ActionsSoundDialog(int row, Action action, QWidget *parent = 0);
	~ActionsSoundDialog();
signals:
	void update(int, Action);
private:
	Ui::ActionsSoundDialog *ui;
	int row;

	void closeEvent(QCloseEvent*);
};

#endif // ACTIONSSOUNDDIALOG_H
