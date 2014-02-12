#ifndef KACHNATRACKER_H
#define KACHNATRACKER_H

#include <QMainWindow>
#include <QLabel>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "util.cpp"
#include "detectordialog.h"

namespace Ui {
class kachnatracker;
}

using namespace cv;

class kachnatracker : public QMainWindow
{
    Q_OBJECT

public:
    explicit kachnatracker(QWidget *parent = 0);
    ~kachnatracker();

    void setOptions(int, int, int);

public slots:
    void updateFrame();
    void updateSettings();

private slots:
    void on_actionOpen_triggered();

    void on_pauseButton_clicked();

    void on_settingsButton_clicked();

private:
    Ui::kachnatracker *ui;
    QLabel *image_label;
    QString fileName;
    DetectorDialog* detectorDialog;

    VideoCapture capture;
    Mat frame;

    unsigned int milliseconds;
    int interval;

    int badFrames;

    QTimer *timer;
    SimpleBlobDetector detector;
    Settings settings;
};

#endif // KACHNATRACKER_H
