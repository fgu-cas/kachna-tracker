#include "experiment.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QDateTime>

#ifdef _WIN32
#include "../cbw.h"
#endif

#include <iostream>

Experiment::Experiment(QObject *parent, QMap<QString, QVariant>  *settings) :
    QObject(parent)
{
    capture.open(settings->value("video/device", 0).toInt());

    detector = new BlobDetector(*settings, capture.get(CV_CAP_PROP_FRAME_HEIGHT), capture.get(CV_CAP_PROP_FRAME_WIDTH));

    timer.setInterval(settings->value("frameInterval", 40).toInt());
    connect(&timer, SIGNAL(timeout()), this, SLOT(processFrame()));

    triggerDistance = settings->value("triggerDistance", 50).toDouble();
#ifdef _WIN32
    float revision = (float) CURRENTREVNUM;
    cbDeclareRevision(&revision);
    cbErrHandling (PRINTALL, DONTSTOP);
    cbDConfigPort (0, FIRSTPORTC, DIGITALOUT);
#endif

    shock.level = 0.4;
    shock.delay = settings->value("shock/EntranceLatency").toInt();
    shock.in_delay = settings->value("shock/InterShockLatency").toInt();
    shock.length = settings->value("shock/ShockDuration").toInt();
    shock.refractory = settings->value("shock/OutsideRefractory").toInt();

    shockState = OUTSIDE;

    stats.goodFrames = 0;
    stats.badFrames = 0;
    stats.entryCount = 0;
    stats.shockCount = 0;
    stats.initialShock = 0;
    lastChange = 0;
}

Experiment::~Experiment(){
    setShock(0);
    delete detector;
}

void Experiment::start(){
    timer.start();
    elapsedTimer.start();
}

void Experiment::stop(){
    setShock(0);
    this->timer.stop();
    capture.release();
    elapsedTime = elapsedTimer.elapsed();
}

void Experiment::processFrame(){

    Mat frame;
    capture >> frame;

    BlobDetector::keyPoints points = detector->detect(&frame);

    double distance = -1;
    bool badFrame = false;
    if (points.rat.size == 0 || points.robot.size == 0){
        stats.badFrames++;
        badFrame = true;
    } else {
        stats.goodFrames++;
        distance = cv::norm(points.rat.pt - points.robot.pt);
    }

    capFrame capframe;
    capframe.keypoints = points;
    capframe.state = shockState;
    capframe.currentLevel = currentLevel;
    capframe.sectors = 0;
    if (distance < triggerDistance && distance != -1){
        capframe.sectors = 1;
    }

    frames.push_back(capframe);


    switch (shockState){
        case OUTSIDE:
            if (!badFrame && distance < triggerDistance){
                shockState = DELAYING;
                stats.entryCount++;
                lastChange = elapsedTimer.elapsed();
            }
            break;
        case DELAYING:
            if (!badFrame){
                if (distance < triggerDistance){
                    if (elapsedTimer.elapsed() > lastChange+shock.delay){
                        shockState = SHOCKING;
                        stats.shockCount++;
                        if (stats.initialShock == 0){
                            stats.initialShock = elapsedTimer.elapsed();
                        }
                        lastChange = elapsedTimer.elapsed();
                        setShock(shock.level);
                    }
                } else {
                    shockState=OUTSIDE;
                }
            }
            break;
        case SHOCKING:
            if (elapsedTimer.elapsed() > lastChange+shock.length){
                setShock(0);
                if (!badFrame){
                    if (distance < triggerDistance){
                        shockState = PAUSE;
                    } else {
                        shockState = REFRACTORY;
                    }
                    lastChange = elapsedTimer.elapsed();
                }
            }
            break;
        case PAUSE:
            if (!badFrame){
                if (distance < triggerDistance){
                    if (elapsedTimer.elapsed() > lastChange+shock.in_delay){
                        shockState = SHOCKING;
                        stats.shockCount++;
                        lastChange = elapsedTimer.elapsed();
                        setShock(shock.level);
                    }
                } else {
                    shockState = REFRACTORY;
                    lastChange = elapsedTimer.elapsed();
                }
            }
            break;
        case REFRACTORY:
            if (elapsedTimer.elapsed() > lastChange+shock.refractory){
                shockState = OUTSIDE;
            }
    }
}

void Experiment::changeShock(double shockLevel){
    if (shockLevel < 0.2){
        shock.level = 0.2;
    } else if (shockLevel > 0.7){
        shock.level = 0.7;
    } else {
        shock.level = shockLevel;
    }
}

Experiment::Update Experiment::getUpdate(){
    Update update;

    if (frames.size() > 0){
        update.keypoints = frames.at(frames.size()-1).keypoints;
    }
    update.stats = stats;

    return update;
}

void Experiment::setShock(double mA){
#ifdef _WIN32
    int level = (int) (mA*10);
    if (level > 7){
        level = 7;
    }
    cbDOut(0, FIRSTPORTC, level);
    currentLevel = level;
#else
    std::cout << "Shock set to " << mA << " @ " << elapsedTimer.elapsed() << std::endl;
#endif
}


QString Experiment::getLog(bool rat){
    QString log;
    log += "%%BEGIN_HEADER\n";
    log += "        %%BEGIN DATABASE_INFORMATION\n";
    log += "                %Date.0 ( "+QDate::currentDate().toString("d.M.yyyy")+" )\n";
    log += "                %Time.0 ( "+QTime::currentTime().toString("h:mm")+" )\n";
    log += "        %%END DATABASE_INFORMATION\n";
    log += "        %%BEGIN SETUP_INFORMATION\n";
    log += "                %TrackerVersion.0 ( Kachna Tracker v1.0 release 04/2014 )\n";
    log += "                %ElapsedTime_ms.0 ( "+QString::number(elapsedTime)+" )\n";
    log += "                %Paradigm.0 ( RobotAvoidance )\n";
    log += QString("                %ShockParameters.0 ( %1 %2 %3 %4 )").arg(QString::number(shock.delay),
                                                                             QString::number(shock.length),
                                                                             QString::number(shock.in_delay),
                                                                             QString::number(shock.refractory));
    log += "                        // %ShockParameters.0 ( EntranceLatency ShockDuration InterShockLatency OutsideRefractory )\n";
/*    log += "                %ArenaDiameter_m.0 ( 0.82 )\n";
    log += "                %TrackerResolution_PixPerCM.0 ( 3.1220 )\n";
    log += "                %ArenaCenterXY.0 ( 127.5 127.5 )\n";*/
    log += "                %Frame.0 ( RoomFrame )\n";
    log += "                %ReinforcedSector.0 ( "+QString::number(triggerDistance)+" )\n";
    log += "                        //%ReinforcedSector.0 ( Radius )\n";
    log += "        %%END SETUP_INFORMATION\n";
    log += "        %%BEGIN RECORD_FORMAT\n";
    log += "                %Sample.0 ( FrameCount 1msTimeStamp RoomX RoomY Sectors State CurrentLevel MotorState Flags FrameInfo )\n";
    log += "                        //Sectors indicate if the object is in a sector. Number is binary coded. Sectors = 0: no sector, Sectors = 1: room sector, Sectors: = 2 arena sector, Sectors: = 3 room and arena sector\n";
    log += "                        //State indicates the Avoidance state: OutsideSector = 0, EntranceLatency = 1, Shock = 2, InterShockLatency = 3, OutsideRefractory = 4, BadSpot = 5\n";
    log += "                        //MotorState indicates: NoMove = 0, MoveCW = positive, MoveCCW = negative\n";
    log += "                        //ShockLevel indicates the level of shock current: NoShock = 0, CurrentLevel = other_values\n";
    log += "                        //FrameInfo indicates succesfuly tracked spots: ReferencePoint * 2^0 + Spot0 * 2^(1+0) + Spot1 * 2^(1+1) + Spot2 * 2^(1+2) .... \n";
    log += "        %%END RECORD_FORMAT\n";
    log += "%%END_HEADER\n";

    for (unsigned i = 0;i < frames.size(); i++){
        log += QString::number(i);
        log += "      ";
        log += QString::number(i*25);
        log += "      ";
        if (rat){
            log += QString::number(frames[i].keypoints.rat.pt.x);
            log += "      ";
            log += QString::number(frames[i].keypoints.rat.pt.y);
        } else {
            log += QString::number(frames[i].keypoints.robot.pt.x);
            log += "      ";
            log += QString::number(frames[i].keypoints.robot.pt.y);
        }
        log += "      ";
        log += QString::number(frames[i].sectors);
        log += "      ";
        log += QString::number(frames[i].state);
        log += "      ";
        log += QString::number(frames[i].currentLevel);
        log += "      *      *      *\n";
    }

    return log;
}
