#ifndef KACHNATRACKER_H
#define KACHNATRACKER_H

#include <memory>
#include <QMainWindow>
#include <QSettings>
#include <QCloseEvent>
#include "configwindow.h"
#include "experiment.h"
#include "detector.h"

#define MAJOR_VERSION 1
#define MINOR_VERSION 0

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

public slots:

    void onConfigurationUpdated(Settings settings);
    void on_actionConfigure_triggered();

    void on_actionImportConfig_triggered();

    void on_actionExportConfig_triggered();

    void on_startButton_clicked();

    void saveTracks();

    void updateTick();
    void requestUpdate();

    void on_actionSave_screenshot_triggered();

private:
    void reset();
    void experimentTimeout();

    void loadSettings(QString);

    void closeEvent(QCloseEvent *);

    configWindow configWin;
    Settings currentSettings;
    QTimer updateTimer;
    QElapsedTimer elapsedTimer;
    bool experimentEnded;


    Ui::kachnatracker *ui;
    std::unique_ptr<QSettings> appSettings;
    std::unique_ptr<Experiment> experiment;
    QPixmap pixmap;
    Detector::keyPoints lastKeypoints;
};

#endif // KACHNATRACKER_H
