#include "abstracttrigger.h"

AbstractTrigger::AbstractTrigger(QObject *parent) : QObject(parent)
{
}

void AbstractTrigger::setEnabled(bool enabled){
    this->enabled = enabled;
}

bool AbstractTrigger::isEnabled(){
    return enabled;
}
