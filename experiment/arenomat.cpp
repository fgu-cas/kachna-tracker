#include "arenomat.h"
#include <QTimer>

#include <QDebug>

enum COMMANDS {
    CHECK = 10,
    SHOCK_LEVEL = 20,
    SHOCK_LENGTH = 21,
    SHOCK_DELAY = 22,
    LED = 30
};


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

void Arenomat::setLed(bool state){
    QByteArray command(3, 0x00);
    command[0] = LED;
    command[1] = state;

    serial.write(command);
}

void Arenomat::setShock(double level){
    QByteArray command(3, 0x00);
    command[0] = SHOCK_LEVEL;
    command[1] = (uint) (level*10);

    serial.write(command);
}

void Arenomat::setShockLength(int length){
    QByteArray command;
    command.append(SHOCK_LENGTH);
    command.append(length & 0xff);
    command.append(length >> 8 & 0xff);

    serial.write(command);
}

void Arenomat::setShockDelay(int length){
    QByteArray command;
    command.append(SHOCK_DELAY);
    command.append(length & 0xff);
    command.append(length >> 8 & 0xff);

    serial.write(command);
}
