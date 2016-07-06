#include "dio24.h"
#include "cbw.h"

DIO24::DIO24()
{
    float revision = (float) CURRENTREVNUM;
    cbDeclareRevision(&revision);
    cbErrHandling (PRINTALL, DONTSTOP);
    cbDConfigPort (0, FIRSTPORTB, DIGITALOUT);
    cbDConfigPort (0, FIRSTPORTC, DIGITALOUT);
}

bool DIO24::check(){
    // TODO: Actually check whether board initialized correctly?
    return true;
}

void DIO24::setShock(int level){
    cbDOut(0, FIRSTPORTC, level);
}

void DIO24::setSync(bool state){
    cbDOut(0, FIRSTPORTB, state ? 1 : 0);
}
