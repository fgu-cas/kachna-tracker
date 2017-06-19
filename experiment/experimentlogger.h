#ifndef EXPERIMENTLOGGER_H
#define EXPERIMENTLOGGER_H

#include <QObject>
#include <QLinkedList>
#include "experiment/params.h"
#include "experiment/detector.h"

typedef QMap<QString, QVariant> Settings;

class ExperimentLogger : public QObject
{
	Q_OBJECT
public:
	explicit ExperimentLogger(Shock shock, Arena arena, QList<Area> areas, QList<Counter> counters, QList<Action> actions, QObject *parent = 0);
	void setStart(qint64 timestamp);
	QString get(qint64 elapsedTime);

	public slots:
	void add(ExperimentState);

private:
	Shock shock;
	Arena arena;
	QList<Counter> counters;
	QList<Area> areas;
	QList<Action> actions;

	bool hasRobot = false;
	bool hasAngle = false;

	struct Frame {
		qint64 timestamp;
		DetectedPoint rat;
		DetectedPoint robot;
		int state;
		int shock;
		QList<Counter> counters;
		QList<Area> areas;
		int position;
	};

	QLinkedList<Frame> frames;
	qint64 startTime;

};

#endif // EXPERIMENTLOGGER_H
