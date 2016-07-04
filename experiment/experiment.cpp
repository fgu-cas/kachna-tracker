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
        shock.distance = settings->value("shock/offsetDistance").toInt();
        shock.angle = settings->value("shock/offsetAngle").toInt();
    }

    timer.setInterval(40);
    timer.setTimerType(Qt::PreciseTimer);
    connect(&timer, SIGNAL(timeout()), this, SLOT(processFrame()));

    synchOut = settings->value("output/sync_enabled").toBool();
    synchInv = settings->value("output/sync_inverted").toBool();
    shockOut = settings->value("output/shock").toBool();

    shock.radius = settings->value("shock/triggerDistance").toInt();

    /* MC library init magic */
    if (shockOut || synchOut){
        float revision = (float) CURRENTREVNUM;
        cbDeclareRevision(&revision);
        cbErrHandling (PRINTALL, DONTSTOP);
        cbDConfigPort (0, FIRSTPORTB, DIGITALOUT);
        cbDConfigPort (0, FIRSTPORTC, DIGITALOUT);
    }

    // multiple_reaction = settings->value("faults/multipleReaction").toInt();
    skip_reaction = settings->value("faults/skipReaction").toInt();
    skip_distance = settings->value("faults/skipDistance").toInt();
    skip_timeout = settings->value("faults/skipTimeout").toInt();

    arena.x = settings->value("arena/X").toInt();
    arena.y = settings->value("arena/Y").toInt();
    arena.size = settings->value("arena/size").toDouble();
    arena.radius = settings->value("arena/radius").toInt();

    shockLevel = 0;
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
    doShock(0);
}

void Experiment::start(){
    timer.start();
    elapsedTimer.start();
    logger.reset(new ExperimentLogger(QDateTime::currentMSecsSinceEpoch(), shock, arena));
}

void Experiment::stop(){
    doShock(0);
    this->timer.stop();
    capture.release();
    finishedTime = elapsedTimer.elapsed();
}

bool Experiment::isRunning(){
    return this->timer.isActive();
}

void Experiment::processFrame(){
    // Output sync signal
    if (synchOut) cbDOut(0, FIRSTPORTB, synchInv ? 0 : 1);

    //Start processing the frame

    Mat frame;
    bool read = capture.read(frame);
    if ( !read && !isLive ){ //We've reached the end of the file
        kachnatracker* mainwindow = dynamic_cast<kachnatracker*>(parent());
        mainwindow->experimentTimeout();
        return;
    }
    qint64 timestamp = elapsedTimer.elapsed();
    lastFrame = frame;

    Detector::pointPair points = detector->find(&frame);


    if (skip_reaction == 1){
       if (lastPoints.rat.size == -1 ||
           cv::norm(lastPoints.rat.pt - points.rat.pt) < skip_distance ||
           ratTimer.elapsed() > skip_timeout){
               lastPoints.rat = points.rat;
               ratTimer.start();
       } else {
           points.rat = Detector::Point();
       }

       if (lastPoints.robot.size == -1 ||
           cv::norm(lastPoints.robot.pt - points.robot.pt) < skip_distance ||
           robotTimer.elapsed() > skip_timeout){
               lastPoints.robot = points.robot;
               robotTimer.start();
       } else {
           points.robot = Detector::Point();
       }
   }

    lastKeypoints = points;

    double distance = -1;
    bool badFrame = false;
    if (points.rat.size == -1 || points.robot.size == -1){
        stats.badFrames++;
        badFrame = true;
    } else {
        stats.goodFrames++;

        Point2f shock_point = points.robot.pt;
        if (shockIsOffset){
            // !!!
            shock_point.x += shock.distance * sin((points.robot.angle + shock.angle)*CV_PI/180);
            shock_point.y -= shock.distance * cos((points.robot.angle + shock.angle)*CV_PI/180);
        }
        distance = cv::norm(points.rat.pt - shock_point);
    }

    int sectors = 0;
    if (distance < shock.radius && distance != -1){
        sectors = 1;
    }

    switch (shockState){
        case OUTSIDE:
            if (!badFrame && distance < shock.radius){
                shockState = DELAYING;
                stats.entryCount++;
                lastChange = elapsedTimer.elapsed();
            }
            break;
        case DELAYING:
            if (!badFrame){
                if (distance < shock.radius){
                    if (elapsedTimer.elapsed() > lastChange+shock.delay){
                        shockState = SHOCKING;
                        stats.shockCount++;
                        if (stats.initialShock == 0){
                            stats.initialShock = elapsedTimer.elapsed();
                        }
                        lastChange = elapsedTimer.elapsed();
                        doShock(shockLevel);
                    }
                } else {
                    shockState=OUTSIDE;
                }
            }
            break;
        case SHOCKING:
            if (elapsedTimer.elapsed() > lastChange+shock.length){
                doShock(0);
                if (!badFrame){
                    if (distance < shock.radius){
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
                if (distance < shock.radius){
                    if (elapsedTimer.elapsed() > lastChange+shock.in_delay){
                        shockState = SHOCKING;
                        stats.shockCount++;
                        lastChange = elapsedTimer.elapsed();
                        doShock(shockLevel);
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

    logger->add(points.rat, sectors, shockState, shockState == SHOCKING ? currentShockLevel : 0, timestamp);
    logger->add(points.robot, sectors, shockState, shockState == SHOCKING ? currentShockLevel : 0, timestamp);

    if (synchOut) cbDOut(0, FIRSTPORTB, synchInv ? 1 : 0);
}

void Experiment::setShockLevel(int level){
    if (level < 2){
        shockLevel = 0;
    } else if (level > 7){
        shockLevel = 7;
    } else {
        shockLevel = level;
    }
}

Experiment::Update Experiment::getUpdate(){
    Update update;

    update.keypoints = lastKeypoints;
    update.stats = stats;
    update.frame = lastFrame;

    return update;
}

void Experiment::doShock(int level){
    if (shockOut){
        cbDOut(0, FIRSTPORTC, level);
        currentShockLevel = level;
    }
}

QString Experiment::getLog(bool rat){
    Detector::CLASS_ID id;
    if (rat){
        id = Detector::RAT;
    } else {
        id = Detector::ROBOT;
    }

    return logger->get(id, finishedTime);
}
