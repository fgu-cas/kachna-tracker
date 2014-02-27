#ifndef KACHNATRACKER_H
#define KACHNATRACKER_H

#include <QMainWindow>
#include <QSettings>
#include <QCloseEvent>
#include "configwindow.h"
#include "experiment.h"
#include "blobdetector.h"

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
private slots:
    void on_actionConfigure_triggered();

    void on_actionImportConfig_triggered();

    void on_actionExportConfig_triggered();

    void on_actionDebug_triggered();

    void on_startButton_clicked();

    void updateTick();
    void requestUpdate();

private:
    void reset();
    void experimentEnded();

    void closeEvent(QCloseEvent *);

    Ui::kachnatracker *ui;
    QSettings *appSettings;
    Experiment *experiment;
    QPixmap pixmap;

    configWindow configWin;

    QTimer experimentTimer;
    QTimer updateTimer;

    BlobDetector::keyPoints lastKeypoints;
};

#endif // KACHNATRACKER_H
