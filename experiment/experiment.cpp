#include "experiment.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "action.h"

#include <QDateTime>

#include "cbw.h"

Experiment::Experiment(QObject *parent, QMap<QString, QVariant>  *settings) :
    QObject(parent)
{
    int deviceIndex = settings->value("video/device").toInt();
    if (deviceIndex == -1){
       capture.open(settings->value("video/filename").toString().toStdString());
       isLive = false;
    } else {
        capture.open(deviceIndex);
       isLive = true;
    }

    if (settings->value("tracking/type").toInt() == 0){
        detector.reset(new DetectorThreshold(*settings, capture.get(CV_CAP_PROP_FRAME_HEIGHT), capture.get(CV_CAP_PROP_FRAME_WIDTH)));
        shockIsOffset = false;
    } else {
        detector.reset(new DetectorColor(*settings, capture.get(CV_CAP_PROP_FRAME_HEIGHT), capture.get(CV_CAP_PROP_FRAME_WIDTH)));
        shockIsOffset = true;
        shockDistance = settings->value("shock/offsetDistance").toInt();
        shockAngle = settings->value("shock/offsetAngle").toInt();
    }

    timer.setInterval(40);
    timer.setTimerType(Qt::PreciseTimer);
    connect(&timer, SIGNAL(timeout()), this, SLOT(processFrame()));

    doSynch = settings->value("output/sync_enabled").toBool();
    synchInv = settings->value("output/sync_inverted").toBool();
    doShock = settings->value("output/shock").toBool();

    triggerDistance = settings->value("shock/triggerDistance").toInt();

    /* MC library init magic */
    if (doShock || doSynch){
        float revision = (float) CURRENTREVNUM;
        cbDeclareRevision(&revision);
        cbErrHandling (PRINTALL, DONTSTOP);
        cbDConfigPort (0, FIRSTPORTB, DIGITALOUT);
        cbDConfigPort (0, FIRSTPORTC, DIGITALOUT);
    }

    arena.x = settings->value("arena/X").toInt();
    arena.y = settings->value("arena/Y").toInt();
    arena.size = settings->value("arena/size").toDouble();
    arena.radius = settings->value("arena/radius").toInt();

    shock.level = 0;
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
}

void Experiment::start(){
    timer.start();
    elapsedTimer.start();
    startTimestamp = QDateTime::currentMSecsSinceEpoch();
}

void Experiment::stop(){
    setShock(0);
    this->timer.stop();
    capture.release();
    elapsedTime = elapsedTimer.elapsed();
}

bool Experiment::isRunning(){
    return this->timer.isActive();
}

void Experiment::processFrame(){
    // Output sync signal
    if (doSynch) cbDOut(0, FIRSTPORTB, synchInv ? 0 : 1);

    //Start processing the frame
    capFrame capframe;

    Mat frame;
    bool read = capture.read(frame);
    if ( !read && !isLive ){ //We've reached the end of the file
        kachnatracker* mainwindow = dynamic_cast<kachnatracker*>(parent());
        mainwindow->experimentTimeout();
        return;
    }
    capframe.timestamp = elapsedTimer.elapsed();
    lastFrame = frame;

    Detector::pointPair points = detector->find(&frame);

    double distance = -1;
    bool badFrame = false;
    if (points.rat.size == 0 || points.robot.size == 0){
        stats.badFrames++;
        badFrame = true;
    } else {
        stats.goodFrames++;

        Point2f shock_point = points.robot.pt;
        if (shockIsOffset){
            // !!!
            shock_point.x += shockDistance * sin((points.robot.angle + shockAngle)*CV_PI/180);
            shock_point.y -= shockDistance * cos((points.robot.angle + shockAngle)*CV_PI/180);
        }
        distance = cv::norm(points.rat.pt - shock_point);
    }

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
    if (doSynch) cbDOut(0, FIRSTPORTB, synchInv ? 1 : 0);
}

void Experiment::setShock(double shockLevel){
    if (shockLevel < 0.2){
        shock.level = 0;
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
    update.frame = lastFrame;

    return update;
}

void Experiment::setShock(int level){
    if (doShock){
        if (level > 7){
            level = 7;
        } else if (level < 0){
            level = 0;
        }
        cbDOut(0, FIRSTPORTC, level);
        currentLevel = level;
    }
}


QString Experiment::getLog(bool rat){
    QDateTime start;
    start.setMSecsSinceEpoch(startTimestamp);

    QString log;
    log += "%%BEGIN_HEADER\r\n";
    log += "        %%BEGIN DATABASE_INFORMATION\r\n";
    log += "                %Date.0 ( "+start.toString("d.M.yyyy")+" )\r\n";
    log += "                %Time.0 ( "+start.toString("h:mm")+" )\r\n";
    log += "                %MilliTime.0 ( "+QString::number(startTimestamp)+" )\r\n";
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
    log += QString("                %ReinforcedSector.0 ( %1 %2 %3 )\r\n").arg(QString::number(triggerDistance),
                                                                               QString::number(shockDistance),
                                                                               QString::number(shockAngle));
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

    for (unsigned i = 0;i < frames.size(); i++){
        log += QString::number(i+1);
        log += "      ";
        log += QString::number(frames[i].timestamp);
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
        log += "      *      *      *      ";
        if (rat){
            log += QString::number(frames[i].keypoints.rat.angle);
        } else {
            log += QString::number(frames[i].keypoints.robot.angle);
        }
        log += "\r\n";
    }

    return log;
}
