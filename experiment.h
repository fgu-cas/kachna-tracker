#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QElapsedTimer>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "blobdetector.h"


using namespace cv;

class Experiment : public QObject
{
    Q_OBJECT
public:
    explicit Experiment(QObject *parent = 0, QMap<QString, QVariant> *settings = 0);
    ~Experiment();
    QString getLog();

private:
    VideoCapture *capture;
    BlobDetector *detector;

    std::vector<Point2f> ratFrame;
    std::vector<Point2f> robotFrame;

    Mat trackingMat;

    QTimer timer;
    QElapsedTimer elapsedTimer;
    int interval;
    unsigned counter;

signals:
    void renderKeypoints(BlobDetector::keyPoints keyPoints);
    void experimentEnd();

public slots:
    void processFrame();
    void start();
    void stop();

};

#endif // EXPERIMENT_H
