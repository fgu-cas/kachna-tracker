#ifndef DUMMYHARDWARE_H
#define DUMMYHARDWARE_H
#include <QObject>
#include "abstracthardware.h"

class DummyHardware : public AbstractHardware
{
    Q_OBJECT
public:
    DummyHardware();

    bool check();
    void setShock(int level);
    void setSync(bool state);
};

#endif // DUMMYHARDWARE_H
