#pragma once
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>

class Logger : public QObject
{
	Q_OBJECT
public:
	Logger();

	enum LEVEL {
		ERROR,
		WARN,
		INFO,
		DEBUG
	};

	struct Message {
		int level;
		QString text;
		QDateTime time;
	};

	void log(QString message, LEVEL level = DEBUG);
	QList<Message> getMessages();
	QString format(Message message);
	void write(QString filename);
signals:
	void messageLogged(Message);

private:
	QList<Message> messages;
};

