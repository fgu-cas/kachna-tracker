#include "counterwindow.hpp"
#include "experiment/experiment.h"
#include <QHBoxLayout>
#include <QLabel>

CounterWindow::CounterWindow(QWidget * parent) : QDialog(parent) {
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);
}

CounterWindow::~CounterWindow() {
}

void CounterWindow::updateView(ExperimentState state) {
	if (!bars.isEmpty()) {
		for (Counter counter : state.counters) {
			QProgressBar* bar = bars.value(counter.id, 0);
			if (bar != 0) {
				bar->setValue(counter.value);
			}
		}
	}
	else {
		for (Counter counter : state.counters) {
			QHBoxLayout* layout = new QHBoxLayout;

			QLabel* label = new QLabel();
			label->setTextFormat(Qt::RichText);
			/*if (counter.active) {
				label->setText(QString("<b>%1</b>").arg(counter.id)));
			}
			else {*/
			label->setText(counter.id);


			QProgressBar* bar = new QProgressBar;
			bar->setMaximum(counter.limit);
			bar->setValue(counter.value);
			bar->setTextVisible(true);
			bar->setFormat("%v/%m");

			layout->addWidget(label);
			layout->addWidget(bar);

			layout->setStretch(0, 0);
			layout->setStretch(1, 1);

			ui.layout->addLayout(layout);

			bars.insert(counter.id, bar);
		}
	}
}
