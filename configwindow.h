#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include "opencv2/core/core.hpp"

#include <QTabWidget>
#include <QMap>

#include <QPixmap>
#include <QPainter>

#include "blobdetector.h"

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
    void on_testButton_clicked();
    void on_refreshTrackingButton_clicked();

private:
    Ui::configWindow *ui;

    Mat capturedFrame;
};

#endif // CONFIGWINDOW_H
