#pragma once
#include <QDialog>

#include "Logger.h"
#include "ui_logwindow.h"

class LogWindow : public QDialog {
	Q_OBJECT

public:
	LogWindow(Logger * logger, QWidget * parent);
	~LogWindow();

	void addMessage(Logger::Message message);

	void saveLog();

private:
	Ui::LogWindow ui;
	Logger* logger;

	private slots:
	void setLevel(int level);
};
