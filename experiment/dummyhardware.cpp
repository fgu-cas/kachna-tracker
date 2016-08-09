#include "dummyhardware.h"
#include <QDebug>

DummyHardware::DummyHardware()
{

}

bool DummyHardware::check(){
    qDebug() << "[HW] Hardware checked alright.";
    return true;
}

void DummyHardware::setShock(int level){
    qDebug() << "[HW] Setting shock to " << level << " mA!";
}

void DummyHardware::setSync(bool state){
    return;
}

void DummyHardware::shutdown(){
    qDebug() << "[HW] Shutting down!";
}
