#ifndef DIO24_H
#define DIO24_H

#include <QObject>
#include <QTimer>
#include "abstracthardware.h"

class DIO24 : public AbstractHardware
{
public:
    DIO24(Logger* logger);
    bool check();
    void setShock(int level);
    void setSync(bool state);
    void shutdown();

	static bool isPresent();

private:
	Logger* logger = 0;
	int state = 0;
};

#endif // DIO24_H
