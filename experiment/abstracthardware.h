#ifndef ABSTRACTHARDWARE_H
#define ABSTRACTHARDWARE_H

#include <QObject>
#include "params.h"
#include "Logger.h"

class AbstractHardware : public QObject
{
	Q_OBJECT
public:
	explicit AbstractHardware(Logger* logger);

	virtual bool check() = 0;
	virtual void setShock(int level) = 0;
	virtual void setSync(bool state) = 0;
	virtual void shutdown() = 0;

signals:

	public slots :

protected:
	Logger* logger;
};

#endif // ABSTRACTHARDWARE_H
