#ifndef DUMMYHARDWARE_H
#define DUMMYHARDWARE_H
#include <QObject>
#include "Logger.h"
#include "abstracthardware.h"

class DummyHardware : public AbstractHardware
{
    Q_OBJECT
public:
    DummyHardware(Logger*);

    bool check();
    void setShock(int level);
    void setSync(bool state);
    void shutdown();
private:
	Logger* logger;
};

#endif // DUMMYHARDWARE_H
