#ifndef ARENOMAT_H
#define ARENOMAT_H

#include <QString>
#include <QtSerialPort/QtSerialPort>
#include <QByteArray>

class Arenomat : public QObject
{
    Q_OBJECT
public:
    Arenomat(QString port);

    bool check();
    void setLed(bool state);
    void setShock(double level);
    void setShockLength(int length);
    void setShockDelay(int length);

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
