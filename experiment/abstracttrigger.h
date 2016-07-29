#ifndef ABSTRACTTRIGGER_H
#define ABSTRACTTRIGGER_H

#include <QObject>
#include <QString>
#include "experiment.h"

class AbstractTrigger : public QObject
{
    Q_OBJECT
public:
    explicit AbstractTrigger(QObject *parent = 0);
    QString getTrigger();

signals:

public slots:
private:
    QString trigger;
};

#endif // ABSTRACTTRIGGER_H
