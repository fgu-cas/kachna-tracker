#include "experimentlogger.h"
#include <QDateTime>

ExperimentLogger::ExperimentLogger(Shock shock, Arena arena,  QObject *parent) : QObject(parent)
{
    this->shock = shock;
    this->arena = arena;
}

void ExperimentLogger::setStart(qint64 timestamp){
    this->startTime = timestamp;
}

void ExperimentLogger::add(Detector::Point point, int sectors, int state, int shock, qint64 timestamp){
    Frame frame;
    frame.point = point;
    frame.sectors = sectors;
    frame.state = state;
    frame.shock = shock;
    frame.timestamp = timestamp;
    frames.append(frame);
}

QString ExperimentLogger::get(Detector::CLASS_ID id, qint64 elapsedTime){
    QDateTime start;
    start.setMSecsSinceEpoch(startTime);

    QString log;
    log += "%%BEGIN_HEADER\r\n";
    log += "        %%BEGIN DATABASE_INFORMATION\r\n";
    log += "                %Date.0 ( "+start.toString("d.M.yyyy")+" )\r\n";
    log += "                %Time.0 ( "+start.toString("h:mm")+" )\r\n";
    log += "                %MilliTime.0 ( "+QString::number(startTime)+" )\r\n";
    log += "        %%END DATABASE_INFORMATION\r\n";
    log += "        %%BEGIN SETUP_INFORMATION\r\n";
    log += "                %TrackerVersion.0 ( Kachna Tracker v1.0 release 04/2014 )\r\n";
    log += "                %ElapsedTime_ms.0 ( "+QString::number(elapsedTime)+" )\r\n";
    log += "                %Paradigm.0 ( RobotAvoidance )\r\n";
    log += QString("                %ShockParameters.0 ( %1 %2 %3 %4 )\r\n").arg(QString::number(shock.delay),
                                                                             QString::number(shock.length),
                                                                             QString::number(shock.in_delay),
                                                                             QString::number(shock.refractory));
    log += "                        // %ShockParameters.0 ( EntranceLatency ShockDuration InterShockLatency OutsideRefractory )\r\n";
    log += "                %ArenaDiameter_m.0 ( "+QString::number(arena.size)+" )\r\n";
    log += "                %TrackerResolution_PixPerCM.0 ( "+QString::number((2*arena.radius)/(arena.size*100))+" )\r\n";
    log += QString("                %ArenaCenterXY.0 ( %1 %2 )\r\n").arg(QString::number(arena.x), QString::number(arena.y));
    log += "                %Frame.0 ( RoomFrame )\r\n";
    /*log += QString("                %ReinforcedSector.0 ( %1 %2 %3 )\r\n").arg(QString::number(shock.radius),
                                                                               QString::number(shock.distance),
                                                                               QString::number(shock.angle));*/
    log += "                        //%ReinforcedSector.0 ( Radius Distance Angle)\r\n";
    log += "        %%END SETUP_INFORMATION\r\n";\
    log += "        %%BEGIN RECORD_FORMAT\r\n";
    log += "                %Sample.0 ( FrameCount 1msTimeStamp RoomX RoomY Sectors State CurrentLevel MotorState Flags FrameInfo Angle)\r\n";
    log += "                        //Sectors indicate if the object is in a sector. Number is binary coded. Sectors = 0: no sector, Sectors = 1: room sector, Sectors: = 2 arena sector, Sectors: = 3 room and arena sector\r\n";
    log += "                        //State indicates the Avoidance state: OutsideSector = 0, EntranceLatency = 1, Shock = 2, InterShockLatency = 3, OutsideRefractory = 4, BadSpot = 5\r\n";
    log += "                        //MotorState indicates: NoMove = 0, MoveCW = positive, MoveCCW = negative\r\n";
    log += "                        //ShockLevel indicates the level of shock current: NoShock = 0, CurrentLevel = other_values\r\n";
    log += "                        //FrameInfo indicates succesfuly tracked spots: ReferencePoint * 2^0 + Spot0 * 2^(1+0) + Spot1 * 2^(1+1) + Spot2 * 2^(1+2) .... \r\n";
    log += "        %%END RECORD_FORMAT\r\n";
    log += "%%END_HEADER\r\n";

    int i = 1;
    auto it = frames.begin();
    while (it != frames.end()){
        Frame frame = (*it);
        if (frame.point.class_id == id){
            log += QString::number(i);
            log += "      ";
            log += QString::number(frame.timestamp);
            log += "      ";
            log += QString::number(frame.point.pt.x);
            log += "      ";
            log += QString::number(frame.point.pt.y);
            log += "      ";
            log += QString::number(frame.sectors);
            log += "      ";
            log += QString::number(frame.state);
            log += "      ";
            log += QString::number(frame.shock);
            log += "      *      *      *      ";
            log += QString::number(frame.point.angle);
            log += "\r\n";
            i++;
        }
        it++;
    }

    return log;
}
