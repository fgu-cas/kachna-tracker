#include "arenomat.h"
#include <QTimer>

enum COMMANDS {
	CHECK = 10,
	SHUTDOWN = 11,
	SHOCK_LEVEL = 20,
	LED = 30,
	TURNTABLE_SPEED = 40,
	TURNTABLE_DIRECTION = 41,
	UPDATE_P = 42,
	UPDATE_I = 43,
	UPDATE_D = 44,
	SET_PWM = 45,
	FEED = 50,
	POSITION = 60
};

Arenomat::Arenomat(Logger* logger) : AbstractHardware(logger) {
	Arenomat(logger, QSerialPortInfo::availablePorts().first().portName());
}

Arenomat::Arenomat(Logger* logger, QString port) : AbstractHardware(logger)
{
	connect(&serial, SIGNAL(readyRead()), this, SLOT(handleData()));
	serial.setPortName(port);
	serial.open(QIODevice::ReadWrite);
	logger->log("Arenomat connection initialized.");
}

Arenomat::~Arenomat() {
	serial.close();
	logger->log("Arenomat connection terminated.");
}

void Arenomat::handleData() {
	buffer.append(serial.readAll());
	if (buffer.size() >= 3) {
		QByteArray message = buffer.left(3);
		buffer.remove(0, 3);
		handleMessage(message);
	}
}

void Arenomat::handleMessage(QByteArray message) {
	emit messageReceived(message);
	if (message == "OK.") {
		emit checkOkay();
	} else if (message == "YA."){
		// command alright
	} else if (message.at(2) == '!') {
		emit(error());
	}
	else {
		unsigned char l = message[0];
		unsigned char h = message[1];
		unsigned tmp = l | (h << 8);
		result = static_cast<int>(tmp);
		emit numberReceived();
	}
}

bool Arenomat::check() {
	QTimer timeout;
	timeout.setSingleShot(true);

	QEventLoop loop;
	loop.connect(this, SIGNAL(checkOkay()), SLOT(quit()));
	loop.connect(&timeout, SIGNAL(timeout()), SLOT(quit()));

	QByteArray command(3, 0x00);
	command[0] = CHECK;
	serial.write(command);
	timeout.start(1000);
	loop.exec();

	// If timer is still active, then we got the response in time,
	// and the connection works okay.
	return timeout.isActive();
}

void Arenomat::setShock(int level) {
	QByteArray command(3, 0x00);
	command[0] = SHOCK_LEVEL;
	command[1] = level;

	serial.write(command);

	logger->log(QString("{HW} Shock set to 0.%1 mA").arg(level));
}

void Arenomat::setSync(bool state) {
	state = state;
}

void Arenomat::shutdown() {
	QByteArray command(3, 0x00);
	command[0] = SHUTDOWN;

	serial.write(command);
	logger->log("{HW} Shutdown issued");
}

void Arenomat::toggleLight() {
	QByteArray command(3, 0x00);
	command[0] = LED;
	command[1] = !currentLight;

	serial.write(command);
	logger->log(QString("{HW} Light set to %1").arg(!currentLight));

	currentLight = !currentLight;
}

void Arenomat::setLight(bool state) {
	QByteArray command(3, 0x00);
	command[0] = LED;
	command[1] = state;

	serial.write(command);
	logger->log(QString("{HW} Light set to %1").arg(state));

	currentLight = state;
}

void Arenomat::setTurntableDirection(int direction) {
	if (direction >= 0 && direction <= 2) {
		QByteArray command(3, 0x00);
		command[0] = TURNTABLE_DIRECTION;
		command[1] = (uint)direction;

		serial.write(command);
	}
	// TODO: humanize enum vals
	logger->log(QString("{HW} Direction set to mode %1").arg(direction));
}

void Arenomat::setTurntableSpeed(int speed) {
	QByteArray command;
	command.append(TURNTABLE_SPEED);
	command.append(speed & 0xff);
	command.append(speed >> 8 & 0xff);

	serial.write(command);
}

void Arenomat::setPIDValue(int constant, double value) {
	QByteArray command;
	switch (constant) {
	case 0:
		command.append(UPDATE_P);
		break;
	case 1:
		command.append(UPDATE_I);
		break;
	case 2:
		command.append(UPDATE_D);
		break;
	default:
		return;
	}
	int value_fixed = value * 10000;
	command.append(value_fixed & 0xff);
	command.append(value_fixed >> 8 & 0xff);

	serial.write(command);
}

void Arenomat::setTurntablePWM(uint8_t pwm) {
	QByteArray command(3, 0x00);
	command[0] = SET_PWM;
	command[1] = pwm;

	serial.write(command);
	logger->log(QString("{HW} Speed set to %1/255").arg(pwm));
}

void Arenomat::feed() {
	QByteArray command(3, 0x00);
	command[0] = FEED;

	serial.write(command);
	logger->log("{HW} Feeder release issued");
}

int Arenomat::position(){
	QTimer timeout;
	timeout.setSingleShot(true);

	QEventLoop loop;
	loop.connect(this, SIGNAL(numberReceived()), SLOT(quit()));
	loop.connect(&timeout, SIGNAL(timeout()), SLOT(quit()));

	QByteArray command(3, 0x00);
	command[0] = POSITION;
	serial.write(command);
	timeout.start(1000);
	loop.exec();

	if (!timeout.isActive()) {
		return -2;
	} else {
		return result;
	}
}
