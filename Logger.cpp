#include "Logger.h"
#include <QFile>
#include <QTextStream>

Logger::Logger()
{
}

void Logger::log(QString text, LEVEL level)
{
	Message message;
	message.text = text;
	message.level = level;
	message.time = QDateTime::currentDateTime();
	messages.append(message);

	emit messageLogged(message);
}

QList<Logger::Message> Logger::getMessages()
{
	return messages;
}

QString Logger::format(Message message)
{
	QString level = "";
	switch (message.level)
	{
	case Logger::ERROR:
		level = "ERROR";
		break;
	case Logger::WARN:
		level = "WARN";
		break;
	case Logger::INFO:
		level = "INFO";
		break;
	case Logger::DEBUG:
		level = "DEBUG";
		break;
	default:
		break;
	}
	return QString("%1 | [%2] %3").arg(message.time.toString("dd/MM/yyyy HH:mm:ss.zzz"))
		.arg(level)
		.arg(message.text);
}

void Logger::write(QString filename)
{
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly)) {
		QTextStream stream(&file);

		for (Message message : messages) {
			stream << format(message);
		}
	}
}