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

    connect(ui->maskXBox, SIGNAL(valueChanged(double)), this, SLOT(valueMaskChanged()));
    connect(ui->maskYBox, SIGNAL(valueChanged(double)), this, SLOT(valueMaskChanged()));
    connect(ui->maskVBox, SIGNAL(valueChanged(double)), this, SLOT(valueMaskChanged()));
    connect(ui->maskHBox, SIGNAL(valueChanged(double)), this, SLOT(valueMaskChanged()));
}

configWindow::~configWindow()
{
    delete ui;
}

QMap<QString, QVariant> configWindow::getSettings()
{
    QMap<QString, QVariant> settings;

    QTime time = ui->lengthEdit->time();
    settings.insert("experimentLength", time.hour()*60*60+time.minute()*60+time.second());

    settings.insert("deviceId", ui->deviceBox->value());
    settings.insert("threshold", ui->threshSpin->value());

    settings.insert("maxArea", ui->maxAreaBox->value());
    settings.insert("minArea", ui->minAreaBox->value());

    settings.insert("maskEnabled", ui->maskEnabledBox->isChecked());
    settings.insert("maskX", ui->maskXBox->value());
    settings.insert("maskY", ui->maskYBox->value());
    settings.insert("maskV", ui->maskVBox->value());
    settings.insert("maskH", ui->maskHBox->value());

    settings.insert("minRat", ui->ratMinSize->value());
    settings.insert("maxRat", ui->ratMaxSize->value());
    settings.insert("minRobot", ui->robotMinSize->value());
    settings.insert("maxRobot", ui->robotMaxSize->value());

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

    ui->maskEnabledBox->setChecked(settings.value("maskEnabled").toBool());
    ui->maskXBox->setValue(settings.value("maskX").toDouble());
    ui->maskYBox->setValue(settings.value("maskY").toDouble());
    ui->maskVBox->setValue(settings.value("maskV").toDouble());
    ui->maskHBox->setValue(settings.value("maskH").toDouble());

    ui->maxAreaBox->setValue(settings.value("maxArea").toDouble());
    ui->minAreaBox->setValue(settings.value("minArea").toDouble());

    ui->ratMinSize->setValue(settings.value("minRat").toDouble());
    ui->ratMaxSize->setValue(settings.value("maxRat").toDouble());
    ui->robotMinSize->setValue(settings.value("minRobot").toDouble());
    ui->robotMaxSize->setValue(settings.value("maxRobot").toDouble());
}

void configWindow::on_testButton_clicked()
{
    VideoCapture capture;
    capture.open(ui->deviceBox->value());

    Mat frame;
    capture >> frame;

    if (!frame.empty()){
        capturedFrame = QPixmap::fromImage(QImage((uchar*) frame.data,
                                                  frame.cols,
                                                  frame.rows,
                                                  frame.step,
                                                  QImage::Format_RGB888));
        ui->videoLabel->setPixmap(capturedFrame);
        valueMaskChanged();
    } else {
        QMessageBox result;
        result.setText("Error! Couldn't retrieve frame.");
        result.exec();
    }

    capture.release();
}

void configWindow::on_refreshTrackingButton_clicked()
{
    VideoCapture capture;
    capture.open(ui->deviceBox->value());

    Mat frame;
    capture >> frame;

    BlobDetector detector(getSettings(), frame.rows, frame.cols);

    BlobDetector::keyPoints keypoints = detector.detect(&frame);

    QPoint rat(keypoints.rat.pt.x, keypoints.rat.pt.y);
    QPoint robot(keypoints.robot.pt.x, keypoints.robot.pt.y);

    std::vector<KeyPoint> allKeypoints = detector.allKeypoints(&frame);
    ui->keypointList->clear();
    for (unsigned i = 0; i < allKeypoints.size(); i++){
        KeyPoint keypoint = allKeypoints[i];
        ui->keypointList->addItem(QString::number(i) + ": " + QString::number(keypoint.pt.x)
                                  + ", " + QString::number(keypoint.pt.y) + " (" +
                                  QString::number(keypoint.size) + ")");
    }


    QPixmap pixmap = QPixmap::fromImage(QImage((uchar*) frame.data,
                                               frame.cols,
                                               frame.rows,
                                               frame.step,
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

void configWindow::valueMaskChanged(){
    QPoint center = QPoint(ui->maskXBox->value(), ui->maskYBox->value());

    QPixmap pixmap = QPixmap(capturedFrame);
    QPainter painter(&pixmap);
    painter.setPen(Qt::magenta);
    painter.drawLine(QPoint(center.x()-5, center.y()-5), QPoint(center.x()+5, center.y()+5));
    painter.drawLine(QPoint(center.x()-5, center.y()+5), QPoint(center.x()+5, center.y()-5));
    painter.drawEllipse(center, (int) ui->maskHBox->value(), (int) ui->maskVBox->value());
    painter.end();

    ui->maskLabel->setPixmap(pixmap);

}
