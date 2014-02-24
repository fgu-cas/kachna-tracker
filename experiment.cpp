#include "experiment.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

Experiment::Experiment(QObject *parent, QMap<QString, QVariant>  *settings) :
    QObject(parent)
{
    connect(this, SIGNAL(update(Experiment::Update)), parent, SLOT(processUpdate(Experiment::Update)));
    connect(this, SIGNAL(renderKeypoints(BlobDetector::keyPoints)), parent, SLOT(renderKeypoints(BlobDetector::keyPoints)));
    connect(this, SIGNAL(experimentEnd()), parent, SLOT(experimentEnded()));

    //TODO: remove this.
    if (settings->value("deviceId").toInt() != 7){
        capture = new VideoCapture(settings->value("deviceId", 0).toInt());
    } else {
        capture = new VideoCapture("/tmp/video.avi");
    }

    detector = new BlobDetector(*settings, capture->get(CV_CAP_PROP_FRAME_HEIGHT), capture->get(CV_CAP_PROP_FRAME_WIDTH));

    timer.setInterval(settings->value("frameInterval", 40).toInt());
    connect(&timer, SIGNAL(timeout()), this, SLOT(processFrame()));
}

Experiment::~Experiment(){
    delete capture;
    delete detector;
}

void Experiment::start(){
    timer.start();
    elapsedTimer.start();
}

void Experiment::stop(){
    this->timer.stop();
    capture->release();
    emit experimentEnd();
}

void Experiment::processFrame(){
    Mat frame;
    *capture >> frame;

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

    }
}


QString Experiment::getLog(){
    QString log;
    for (unsigned i = 0;i < ratFrame.size(); i++){
        log += QString::number(ratFrame[i].x) + ", " + QString::number(ratFrame[i].y) + "\n";
    }
    return log;
}
