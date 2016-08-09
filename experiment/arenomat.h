#ifndef ARENOMAT_H
#define ARENOMAT_H

#include <QString>
#include <QtSerialPort/QtSerialPort>
#include <QByteArray>
#include "abstracthardware.h"

class Arenomat : public AbstractHardware
{
    Q_OBJECT
public:
    Arenomat();
    Arenomat(QString port);

    bool check();
    void setShock(int level);
    void setSync(bool state);
    void shutdown();

    void setLed(bool state);
    void setTurntableDirection(int direction);
    void setTurntableSpeed(int speed);
    void setPIDValue(int constant, double value);
    void setTurntablePWM(uint8_t pwm);

signals:
    void messageReceived(QByteArray message);
    void checkOkay();
    void error();
    void communicationFail();

private:
    QSerialPort serial;
    QByteArray buffer;
    void handleMessage(QByteArray command);

private slots:
    void handleData();
};

#endif // ARENOMAT_H
