#ifndef ACTIONS_AREA_SECTOR_DIALOG_H
#define ACTIONS_AREA_SECTOR_DIALOG_H

#include <QDialog>
#include "params.h"

namespace Ui {
	class AreaSectorDialog;
}

class AreaSectorDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AreaSectorDialog(QWidget *parent, int row, Area area);
	~AreaSectorDialog();

signals:
	void update(int row, Area area);

	private slots:
	void on_angleSlider_sliderMoved(int position);

	void on_angleSpinbox_valueChanged(double arg1);

	void on_rangeSlider_sliderMoved(int position);

	void on_rangeSpinbox_valueChanged(double arg1);

private:
	Ui::AreaSectorDialog *ui;
	int row;

	void closeEvent(QCloseEvent*);

};


#endif // ACTIONS_AREA_SECTOR_DIALOG_H
