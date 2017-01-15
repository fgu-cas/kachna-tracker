#pragma once
#include <QWidget>
#include <QDialog>
#include "ui_counterwindow.h"
#include "params.h"
#include "experiment.h"

#include <QProgressBar>

class CounterWindow : public QDialog {
	Q_OBJECT

public:
	CounterWindow(QWidget * parent = Q_NULLPTR);
	~CounterWindow();

	public slots:
	void updateView(ExperimentState state);

private:
	Ui::CounterWindow ui;
	QMap<QString, QProgressBar*> bars;
};
