#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QElapsedTimer>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "params.h"
#include "mainwindow.h"
#include "detector_threshold.h"
#include "experimentlogger.h"
#include "abstracthardware.h"

using namespace cv;

class Experiment : public QObject
{
    Q_OBJECT
public:
    explicit Experiment(QObject *parent = 0, QMap<QString, QVariant> *settings = 0);
    ~Experiment();
    bool isRunning();
    QString getLog(bool rat);

    struct Stats {
        int goodFrames;
        int badFrames;
        int shockCount;
        int entryCount;
        qint64 initialShock;
    };

    struct Update {
        Detector::pointPair keypoints;
        Stats stats;
        Mat frame;
    };

    Update getUpdate();
    QList<Area> getAreas();


private:
    // System
    Stats stats;
    std::unique_ptr<ExperimentLogger> logger;

    QTimer timer;
    QElapsedTimer elapsedTimer;
    qint64 finishedTime;

    Detector::pointPair lastKeypoints;

    bool ratRobot;

    bool shockOut;
    bool synchOut;
    bool synchInv;

    // Capture
    VideoCapture capture;
    bool isLive;
    Mat lastFrame;

    // Detection
    std::unique_ptr<Detector> detector;

    int multiple_reaction;
    int skip_reaction;
    int skip_distance;
    int skip_timeout;
    Detector::pointPair lastPoints;
    QElapsedTimer ratTimer;
    QElapsedTimer robotTimer;

    // Shock
    enum shockStates {
        OUTSIDE,
        DELAYING,
        SHOCKING,
        PAUSE,
        REFRACTORY
    } shockState;
    double shockLevel;
    int currentShockLevel = 2;

    Shock shock;

    qint64 lastChange;

    Arena arena;

    QList<Area> areas;
    QList<Counter> counters;
    QList<Action> actions;

    std::unique_ptr<AbstractHardware> hardware;

public slots:
    void start();
    void stop();
    void setShockLevel(int level);

private slots:
    void processFrame();

};

#endif // EXPERIMENT_H
