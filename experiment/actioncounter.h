#ifndef ACTIONCOUNTER_H
#define ACTIONCOUNTER_H

#include <QObject>

class ActionCounter : public QObject
{
    Q_OBJECT
public:
    explicit ActionCounter(QObject *parent = 0);

    bool isActive();

signals:

public slots:
};

#endif // ACTIONCOUNTER_H
