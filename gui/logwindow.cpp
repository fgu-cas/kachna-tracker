#include "logwindow.hpp"
#include <QFileDialog>

LogWindow::LogWindow(Logger* logger, QWidget * parent) : QDialog(parent), logger(logger) {
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);

	for (Logger::Message message : logger->getMessages()) {
		addMessage(message);
	}

	connect(logger, &Logger::messageLogged, this, &LogWindow::addMessage);
	connect(ui.levelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setLevel(int)));
	connect(ui.saveButton, &QPushButton::clicked, this, &LogWindow::saveLog);
}

LogWindow::~LogWindow() {

}

void LogWindow::addMessage(Logger::Message message) {
	if (message.level <= ui.levelComboBox->currentIndex()) {
		QListWidgetItem* item = new QListWidgetItem(logger->format(message));
		switch (message.level)
		{
		case Logger::ERROR:
			item->setBackgroundColor(Qt::red);
			break;
		case Logger::WARN:
			item->setBackgroundColor(Qt::yellow);
			break;
		case Logger::INFO:
			// do nothing
			break;
		case Logger::DEBUG:
			item->setBackgroundColor(Qt::gray);
			break;
		default:
			break;
		}
		ui.listWidget->addItem(item);
		ui.listWidget->scrollToBottom();
	}
}

void LogWindow::setLevel(int level) {
	ui.listWidget->clear();

	for (Logger::Message message : logger->getMessages()) {
		addMessage(message);
	}
}

void LogWindow::saveLog() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save log file"),
		"",
		tr("Files (*.txt)"));

	if (!filename.isEmpty()) {
		logger->write(filename);
	}
}