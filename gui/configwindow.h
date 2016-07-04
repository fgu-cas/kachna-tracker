#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H
#include "ui_configwindow.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <QTabWidget>
#include <QMap>

#include <QTimer>
#include <QPixmap>
#include <QPainter>

#include <QStringListModel>

#include "detector_threshold.h"
#include "detector_color.h"
#include "action.h"
#include "models.h"

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
    double pixelsToMeters(int px);
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

    void trackFrame();

    void trackingParamsChanged();

    void updateTrackingView();

    void on_maskButton_toggled(bool checked);

    void on_trackingCombobox_currentIndexChanged(int index);

    void on_resolutionBox_toggled(bool checked);

    void captureResolutionChanged();

    void addAction();
    void addAction(QString trigger, Action *action);
    void addArea();
    void addArea(QString title, AreaSectorSettings settings, bool enabled);
    void addArea(QString title, AreaRobotSettings settings, bool enabled);
    void addCounter();
    void addCounter(QString title, double limit, double frequency, bool enabled);
    void removeThisActionRow();
    void removeThisAreaRow();
    void removeThisCounterRow();
    void clearAllActions();

    void actionAreaSetPressed();
    void actionActionSetPressed();

    void triggersChanged(QString id);

private:
    Ui::configWindow *ui;
    std::unique_ptr<Detector> detector;
    void resetDetector();

    Settings compileSettings();
    Settings lastSettings;

    QPixmap testFrame;
    Mat trackingFrame;
    QTimer refreshTimer;
    VideoCapture capture;

    void closeEvent(QCloseEvent*);
    void showEvent(QShowEvent *);
    QString videoFilename;

    QStringListModel actionTriggers;
};

#endif // CONFIGWINDOW_H
