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

using namespace cv;

namespace Ui {
class configWindow;
}

class configWindow : public QTabWidget
{
    Q_OBJECT

public:
    explicit configWindow(QWidget *parent = 0);
    QMap<QString, QVariant> getSettings();
    void setSettings(QMap<QString, QVariant> settings);
    ~configWindow();

signals:
    void configurationUpdated();

private slots:

    void maskValueChanged();
    void on_testButton_clicked();
    void on_refreshTrackingButton_clicked();
    void on_browseButton_clicked();
    void on_checkBox_stateChanged(int arg1);

private:
    Ui::configWindow *ui;

    QPixmap capturedFrame;

    QTimer refreshTimer;
    VideoCapture capture;
};

#endif // CONFIGWINDOW_H
