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

    shockActive = false;
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

    ratFrame.push_back(points.rat);
    robotFrame.push_back(points.robot);

    bool badFrame = false;
    if (points.rat.size == 0 || points.robot.size == 0){
        badFrames++;
        badFrame = true;
    } else {
        goodFrames++;
    }

    if (!badFrame){
        double distance = norm(points.rat.pt - points.robot.pt);

        if (distance < triggerDistance){
            setShock(0.4);
            shockActive = true;
        } else {
            if (shockActive){
                setShock(0);
                shockActive = false;
            }
        }
    }
}

Experiment::Update Experiment::getUpdate(){
    Update update;

    update.keypoints.rat = ratFrame.at(ratFrame.size()-1);
    update.keypoints.robot = robotFrame.at(robotFrame.size()-1);
    update.goodFrames = goodFrames;
    update.badFrames = badFrames;

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
    for (unsigned i = 0;i < ratFrame.size(); i++){
        log += QString::number(ratFrame[i].pt.x) + ", " + QString::number(ratFrame[i].pt.y) + "\n";
    }
    return log;
}
