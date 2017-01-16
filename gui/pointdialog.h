#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QSlider>
#include <QCheckBox>
#include <QSpinBox>

#include "pointselector.h"


class PointDialog : public QDialog
{
	Q_OBJECT
public:
	explicit PointDialog(QWidget *parent = 0);

	void setTitle(QString title);
	void setRange(pointRange range);

signals:
	void rangeChanged(pointRange range);

	public slots:
	void onRevertButtonPressed();
	void onControlsChanged();
	void limitsToggled(bool);

private:
	pointRange getRange();

	QSlider* hueSlider;
	QSlider* hueTolSlider;
	QCheckBox* sizeCheckbox;
	QSpinBox* minSpinbox;
	QSpinBox* maxSpinbox;
};

#endif // COLORDIALOG_H
