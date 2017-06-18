#ifndef ACTIONSSOUNDDIALOG_H
#define ACTIONSSOUNDDIALOG_H

#include <QDialog>

namespace Ui {
	class ActionsSoundDialog;
}

class ActionsSoundDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ActionsSoundDialog(QWidget *parent = 0);
	~ActionsSoundDialog();

	private slots:
	void on_comboBox_currentIndexChanged(int index);

private:
	Ui::ActionsSoundDialog *ui;
};

#endif // ACTIONSSOUNDDIALOG_H
