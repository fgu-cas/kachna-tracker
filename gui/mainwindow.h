#ifndef KACHNATRACKER_H
#define KACHNATRACKER_H

#include <memory>
#include <QMainWindow>
#include <QSettings>
#include <QElapsedTimer>
#include <QCloseEvent>
#include <QImage>
#include "configwindow.h"
#include "Logger.h"
#include "detector.h"
class Experiment;

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

	void receiveUpdate(ExperimentState);

	void drawUpdate();

    void saveTracks();

    void updateTick();
    void experimentTimeout();

    void on_actionSave_screenshot_triggered();

private slots:
    void on_actionAbout_triggered();

    void on_actionVideo_tracking_toggled(bool arg1);

    void on_actionDebug_triggered();

	void on_actionMessage_Log_triggered();

	void showCounterWindow();

private:
    void reset();

    void loadSettings(QString);

    void closeEvent(QCloseEvent *);

	Logger logger;
	configWindow configWin{ &logger };
    Settings currentSettings;
    QTimer updateTimer;
    QElapsedTimer elapsedTimer;
    bool experimentEnded;
    bool isLive;

    Ui::kachnatracker *ui;
    std::unique_ptr<QSettings> appSettings;
    std::unique_ptr<Experiment> experiment;
    QImage trackImage;
    QRect arenaArea;
    Detector::pointPair lastKeypoints;
	ExperimentState lastUpdate;

    bool dirty = false;
    bool showVideo = false;
};

#include "experiment.h"

#endif // KACHNATRACKER_H
