#include "configwindow.h"
#include "ui_configwindow.h"
#include <QMessageBox>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <iostream>

configWindow::configWindow(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::configWindow)
{
    ui->setupUi(this);
}

configWindow::~configWindow()
{
    delete ui;
}

void configWindow::on_testButton_clicked()
{
    //TODO: remove this.
    VideoCapture capture;
    if (ui->deviceBox->value() != 7){
        capture = VideoCapture(ui->deviceBox->value());
    } else {
        capture = VideoCapture("/tmp/video.avi");
    }

    capture >> capturedFrame;

    if (!capturedFrame.empty()){
        QImage image = QImage((uchar*) capturedFrame.data, capturedFrame.cols, capturedFrame.rows, capturedFrame.step, QImage::Format_RGB888);
        ui->videoLabel->setPixmap(QPixmap::fromImage(image));
    } else {
        QMessageBox result;
        result.setText("Error! Couldn't retrieve frame.");
        result.exec();
    }

    capture.release();
}

QMap<QString, QVariant> configWindow::getSettings()
{
    QMap<QString, QVariant> settings;

    QTime time = ui->lengthEdit->time();
    settings.insert("experimentLength", time.hour()*60*60+time.minute()*60+time.second());

    settings.insert("deviceId", ui->deviceBox->value());
    settings.insert("threshold", ui->threshSpin->value());


    settings.insert("ratMinSize", ui->ratMinSize->value());
    settings.insert("ratMaxSize", ui->ratMaxSize->value());
    settings.insert("robotMinSize", ui->robotMinSize->value());
    settings.insert("robotMaxSize", ui->robotMaxSize->value());

    return settings;
}


void configWindow::setSettings(QMap<QString, QVariant> settings){
    int length = settings.value("experimentLength").toInt();
    int s = length%60;
    length /= 60;
    int m = length % 60;
    length /= 60;
    int h = length % 24;

    ui->lengthEdit->setTime(QTime(h, m, s));

    ui->deviceBox->setValue(settings.value("deviceId").toInt());
    ui->threshSpin->setValue(settings.value("threshold").toInt());

    ui->ratMinSize->setValue(settings.value("ratMinSize").toInt());
    ui->ratMaxSize->setValue(settings.value("ratMaxSize").toInt());
    ui->robotMinSize->setValue(settings.value("robotMinSize").toInt());
    ui->robotMaxSize->setValue(settings.value("robotMaxSize").toInt());
}

void configWindow::on_refreshTrackingButton_clicked()
{
    BlobDetector detector(ui->threshSpin->value(),
                          ui->ratMaxSize->value(),
                          ui->ratMinSize->value(),
                          ui->robotMaxSize->value(),
                          ui->robotMinSize->value());

    BlobDetector::keyPoints keypoints = detector.detect(&capturedFrame);

    std::cout << keypoints.rat.pt.x << ", " << keypoints.rat.pt.y << std::endl << std::flush;
    std::cout << keypoints.robot.pt.x << ", " << keypoints.robot.pt.y << std::endl << std::flush;

    QPoint rat(keypoints.rat.pt.x, keypoints.rat.pt.y);
    QPoint robot(keypoints.robot.pt.x, keypoints.robot.pt.y);


    QPixmap pixmap = QPixmap::fromImage(QImage((unsigned char*) capturedFrame.data,
                                               capturedFrame.cols,
                                               capturedFrame.rows,
                                               QImage::Format_RGB888));

    QPainter painter(&pixmap);

    int ratSize = (int)(keypoints.rat.size+0.5);
    painter.setPen(Qt::red);
    painter.drawEllipse(rat, ratSize, ratSize);

    int robotSize = (int)(keypoints.robot.size+0.5);
    painter.setPen(Qt::blue);
    painter.drawEllipse(robot, robotSize, robotSize);

    painter.end();

    ui->trackingLabel->setPixmap(pixmap);
}
