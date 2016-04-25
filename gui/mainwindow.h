#ifndef KACHNATRACKER_H
#define KACHNATRACKER_H

#include <memory>
#include <QMainWindow>
#include <QSettings>
#include <QElapsedTimer>
#include <QCloseEvent>
#include "configwindow.h"
#include "detector.h"
class Experiment;

#define MAJOR_VERSION 3

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
    void experimentTimeout();
    void requestUpdate();

    void on_actionSave_screenshot_triggered();

private slots:
    void on_actionAbout_triggered();

private:
    void reset();

    void loadSettings(QString);

    void closeEvent(QCloseEvent *);

    configWindow configWin;
    Settings currentSettings;
    QTimer updateTimer;
    QElapsedTimer elapsedTimer;
    bool experimentEnded;
    bool isLive;


    Ui::kachnatracker *ui;
    std::unique_ptr<QSettings> appSettings;
    std::unique_ptr<Experiment> experiment;
    QPixmap pixmap;
    Detector::pointPair lastKeypoints;

    bool dirty = false;
};

#include "experiment.h"

#endif // KACHNATRACKER_H
