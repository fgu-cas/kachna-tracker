#include "dio24.h"
#include "../cbw.h"

DIO24::DIO24(Logger* logger) : AbstractHardware(logger)
{
    float revision = (float) CURRENTREVNUM;
    state = cbDeclareRevision(&revision);
    cbErrHandling (PRINTALL, DONTSTOP);
    cbDConfigPort (0, FIRSTPORTB, DIGITALOUT);
    cbDConfigPort (0, FIRSTPORTC, DIGITALOUT);

	logger->log(QString("DIO24 initialized"));
}

bool DIO24::check(){
	return state == 0;
}

void DIO24::setShock(int level){
    cbDOut(0, FIRSTPORTC, level);
	logger->log(QString("{HW} Shock set to 0.%1 mA").arg(level));
}

void DIO24::setSync(bool state){
    cbDOut(0, FIRSTPORTB, state ? 1 : 0);
}

void DIO24::shutdown(){
    cbDOut(0, FIRSTPORTB, 0);
    cbDOut(0, FIRSTPORTC, 0);
	logger->log("{HW} Shutdown issued");
}

bool DIO24::isPresent()
{
	// TODO
	return true;
}
