#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <QTabWidget>
#include <QMap>

#include <QTimer>
#include <QPixmap>
#include <QPainter>

#include "detector.h"

typedef QMap<QString, QVariant> Settings;

using namespace cv;

namespace Ui {
class configWindow;
}

class configWindow : public QTabWidget
{
    Q_OBJECT

public:
    explicit configWindow(QWidget *parent = 0);
    ~configWindow();

signals:
    void configurationUpdated(Settings);

public slots:

    void load(Settings);

    void refreshDevices();
    void maskValueChanged();
    void on_testButton_clicked();
    void on_refreshTrackingButton_clicked();
    void on_browseButton_clicked();
    void on_refreshCheckbox_stateChanged(int arg1);
    void on_deviceCombobox_activated(int);

private slots:
    void on_revertButton_clicked();

    void on_applyButton_clicked();

    void on_okayButton_clicked();

    void on_skipCombo_currentIndexChanged(int index);

    void refreshTracking();

private:
    Ui::configWindow *ui;
    std::unique_ptr<Detector> detector;

    Settings compileSettings();
    Settings lastSettings;

    QPixmap capturedFrame;
    QTimer refreshTimer;
    VideoCapture capture;

    void closeEvent(QCloseEvent*);
    void showEvent(QShowEvent *);
    QString videoFilename;
};

#endif // CONFIGWINDOW_H
