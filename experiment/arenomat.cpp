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
    SET_PWM = 45
};

Arenomat::Arenomat(){
    Arenomat(QSerialPortInfo::availablePorts().first().portName());
}

Arenomat::Arenomat(QString port)
{
    connect(&serial, SIGNAL(readyRead()), this, SLOT(handleData()));
    serial.setPortName(port);
    serial.open(QIODevice::ReadWrite);
}

void Arenomat::handleData(){
    buffer.append(serial.readAll());
    if (buffer.size() >= 3){
        QByteArray message = buffer.left(3);
        buffer.remove(0, 3);
        handleMessage(message);
    }
}

void Arenomat::handleMessage(QByteArray message){
    emit messageReceived(message);
    if (message == "OK."){
        emit checkOkay();
    }
    if (message.at(2) == '!'){
        emit(error());
    }
}

bool Arenomat::check(){
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

void Arenomat::setShock(int level){
    QByteArray command(3, 0x00);
    command[0] = SHOCK_LEVEL;
    command[1] = level;

    serial.write(command);
}

void Arenomat::setSync(bool state){
    state = state;
    // TODO
}

void Arenomat::shutdown(){
    QByteArray command(3, 0x00);
    command[0] = SHUTDOWN;

    serial.write(command);
}

void Arenomat::setLight(bool state){
    QByteArray command(3, 0x00);
    command[0] = LED;
    command[1] = state;

    serial.write(command);
}

void Arenomat::setTurntableDirection(int direction){
    if (direction >= 0 && direction <= 2){
        QByteArray command(3, 0x00);
        command[0] = TURNTABLE_DIRECTION;
        command[1] = (uint) direction;

        serial.write(command);
    }
}

void Arenomat::setTurntableSpeed(int speed){
    QByteArray command;
    command.append(TURNTABLE_SPEED);
    command.append(speed & 0xff);
    command.append(speed >> 8 & 0xff);

    serial.write(command);
}

void Arenomat::setPIDValue(int constant, double value){
    QByteArray command;
    switch (constant){
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
    int value_fixed = value*10000;
    command.append(value_fixed & 0xff);
    command.append(value_fixed >> 8 & 0xff);

    serial.write(command);
}

void Arenomat::setTurntablePWM(uint8_t pwm){
    QByteArray command(3, 0x00);
    command[0] = SET_PWM;
    command[1] = pwm;

    serial.write(command);
}
