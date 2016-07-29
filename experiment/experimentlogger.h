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
    explicit ExperimentLogger(Shock shock, Arena arena, QObject *parent = 0);
    void setStart(qint64 timestamp);
    QString get(Detector::CLASS_ID id, qint64 elapsedTime);

public slots:
    void add(Detector::Point point, int sectors, int state, int shock, qint64 timestamp);

private:
    // to remove
    Shock shock;
    Arena arena;

    struct Frame {
        Detector::Point point;
        int sectors;
        int state;
        int shock;
        qint64 timestamp;
    };

    QLinkedList<Frame> frames;
    qint64 startTime;

};

#endif // EXPERIMENTLOGGER_H
