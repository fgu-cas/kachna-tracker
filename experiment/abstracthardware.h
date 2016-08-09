#ifndef ABSTRACTHARDWARE_H
#define ABSTRACTHARDWARE_H

#include <QObject>
#include "params.h"

class AbstractHardware : public QObject
{
    Q_OBJECT
public:
    explicit AbstractHardware();

    virtual bool check() = 0;
    virtual void setShock(int level) = 0;
    virtual void setSync(bool state) = 0;
    virtual void shutdown() = 0;

signals:

public slots:

private:
};

#endif // ABSTRACTHARDWARE_H
