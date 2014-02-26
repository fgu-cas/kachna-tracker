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
    connect(this, SIGNAL(update(Experiment::Update)), parent, SLOT(processUpdate(Experiment::Update)));
    connect(this, SIGNAL(renderKeypoints(BlobDetector::keyPoints)), parent, SLOT(renderKeypoints(BlobDetector::keyPoints)));
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
    lastFrame = 0;

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

    ratFrame.push_back(points.rat.pt);
    robotFrame.push_back(points.robot.pt);

    bool badFrame = false;
    if (points.rat.size == 0 || points.robot.size == 0){
        emit update(BAD_FRAME);
        badFrame = true;
    } else {
        emit update(GOOD_FRAME);
    }

    if (!badFrame){
        double distance = norm(points.rat.pt - points.robot.pt);

        if (distance < triggerDistance){
            setShock(0.4);
            shockActive = true;
            emit(renderKeypoints(points));
        } else {
            if (shockActive){
                setShock(0);
                shockActive = false;
            }
        }

        if (elapsedTimer.elapsed() > lastFrame + 750){
            emit(renderKeypoints(points));
            lastFrame = elapsedTimer.elapsed();
        }
    }

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
        log += QString::number(ratFrame[i].x) + ", " + QString::number(ratFrame[i].y) + "\n";
    }
    return log;
}
