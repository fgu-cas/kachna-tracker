#include "dummyhardware.h"
#include <QDebug>

DummyHardware::DummyHardware(Logger* logger) : AbstractHardware(logger)
{
	this->logger = logger;
	logger->log("{DUMMY} Hardware initialized.");
}

bool DummyHardware::check() {
	logger->log("{DUMMY} Hardware checked alright.");
	return true;
}

void DummyHardware::setShock(int level) {
	logger->log(QString("{DUMMY} Setting shock to %1 mA!").arg(level));
}

void DummyHardware::setSync(bool state) {
	return;
}

void DummyHardware::shutdown() {
	logger->log("{DUMMY} Shutting down!");
}
