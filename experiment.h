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

    struct Update {
        BlobDetector::keyPoints keypoints;
        int goodFrames;
        int badFrames;
    };
    Update getUpdate();


private:
    void setShock(double mA);

    VideoCapture capture;
    BlobDetector *detector;

    std::vector<KeyPoint> ratPoints;
    std::vector<KeyPoint> robotPoints;

    QTimer timer;
    QElapsedTimer elapsedTimer;

    int goodFrames;
    int badFrames;

    double triggerDistance;

    enum {
        OUT,
        DELAYING,
        SHOCKING,
        PAUSE
    } shockState;
    struct {
        double level;
        int length;
        int delay;
        int in_delay;
        int refractory;
    } shock;
    qint64 lastChange;

signals:
    void experimentEnd();

public slots:
    void start();
    void stop();
    void changeShock(double shock);

private slots:
    void processFrame();

};

#endif // EXPERIMENT_H
