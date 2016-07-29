#ifndef DIO24_H
#define DIO24_H

#include <QObject>
#include <QTimer>
#include "abstracthardware.h"

class DIO24 : public AbstractHardware
{
public:
    DIO24();
    bool check();
    void setShock(int level);
    void setSync(bool state);

private:
};

#endif // DIO24_H
