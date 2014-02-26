#include "experiment.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef _WIN32
#include "../cbw.h"
#endif

#include <iostream>

Experiment::Experiment(QObject *parent, QMap<QString, QVariant>  *settings) :
    QObject(parent)
{
    connect(this, SIGNAL(experimentEnd()), parent, SLOT(experimentEnded()));

    capture.open(settings->value("deviceId", 0).toInt());

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
    shock.delay = settings->value("shockDelay").toInt();
    shock.in_delay = settings->value("shockInterDelay").toInt();
    shock.length = settings->value("shockLength").toInt();
    shock.refractory = settings->value("shockRefractoryPeriod").toInt();

    shockState = OUT;

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
    emit experimentEnd();
}

void Experiment::processFrame(){
    Mat frame;
    capture >> frame;

    BlobDetector::keyPoints points = detector->detect(&frame);
    ratPoints.push_back(points.rat);
    robotPoints.push_back(points.robot);

    double distance = -1;
    bool badFrame = false;
    if (points.rat.size == 0 || points.robot.size == 0){
        stats.badFrames++;
        badFrame = true;
    } else {
        stats.goodFrames++;
        distance = cv::norm(points.rat.pt - points.robot.pt);
    }


    switch (shockState){
        case OUT:
            if (!badFrame && distance < triggerDistance
                    && elapsedTimer.elapsed() > lastChange+shock.refractory){
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
                    shockState=OUT;
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
                        shockState = OUT;
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
                    shockState = OUT;
                    lastChange = elapsedTimer.elapsed();
                }
            }
            break;
    }
}

void Experiment::changeShock(double shockLevel){
    if (shockLevel < 0.2){
        shock.level = 0.2;
    } else if (shockLevel > 0.7){
        shock.level = 0.7;
    }
}

Experiment::Update Experiment::getUpdate(){
    Update update;

    update.keypoints.rat = ratPoints.at(ratPoints.size()-1);
    update.keypoints.robot = robotPoints.at(robotPoints.size()-1);
    update.stats = stats;

    return update;
}

void Experiment::setShock(double mA){
#ifdef _WIN32
    if (mA > 0.7){
        mA = 0.7;
    }
    cbDOut(0, FIRSTPORTC, (int) (mA*10));
#else
    std::cout << "Shock set to " << mA << " @ " << elapsedTimer.elapsed() << std::endl;
#endif
}


QString Experiment::getLog(){
    QString log;
    for (unsigned i = 0;i < ratPoints.size(); i++){
        log += QString::number(ratPoints[i].pt.x) + ", " + QString::number(ratPoints[i].pt.y) + "\n";
    }
    return log;
}
