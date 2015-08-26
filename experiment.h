#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QElapsedTimer>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "detector.h"


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
        Detector::keyPoints keypoints;
        Stats stats;
    };

    Update getUpdate();


private:
    qint64 startTimestamp;
    QString startTime;
    QString startDate;

    void setShock(double mA);

    struct capFrame {
        Detector::keyPoints keypoints;
        int sectors;
        int state;
        int currentLevel;
        qint64 timestamp;
    };

    VideoCapture capture;
    std::unique_ptr<Detector> detector;

    std::vector<capFrame> frames;
    QTimer timer;
    QElapsedTimer elapsedTimer;
    qint64 elapsedTime;

    Stats stats;

    double triggerDistance;

    enum shockStates {
        OUTSIDE,
        DELAYING,
        SHOCKING,
        PAUSE,
        REFRACTORY
    } shockState;
    int currentLevel = 0;

    struct {
        double level;
        int length;
        int delay;
        int in_delay;
        int refractory;
    } shock;
    qint64 lastChange;

    struct {
        int x;
        int y;
        double size;
        int radius;
    } arena;

public slots:
    void start();
    void stop();
    void changeShock(double shock);

private slots:
    void processFrame();

};

#endif // EXPERIMENT_H
