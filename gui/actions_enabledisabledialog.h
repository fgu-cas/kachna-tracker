#ifndef ENABLEDISABLEDIALOG_H
#define ENABLEDISABLEDIALOG_H

#include <QDialog>
#include <QStringListModel>
#include "params.h"
namespace Ui {
	class EnableDisableDialog;
}

class ActionEnableDisableDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ActionEnableDisableDialog(QStringListModel &targets, int row, Action action, QWidget *parent = 0);
	~ActionEnableDisableDialog();
signals:
	void update(int, Action);
private:
	Ui::EnableDisableDialog *ui;
	int row;

	void closeEvent(QCloseEvent*);
};

#endif // ENABLEDISABLEDIALOG_H
