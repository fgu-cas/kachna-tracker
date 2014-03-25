#include "configwindow.h"
#include "ui_configwindow.h"
#include <QMessageBox>
#include <QFileDialog>
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
    connect(ui->maskRadiusBox, SIGNAL(valueChanged(int)), this, SLOT(valueMaskChanged()));

    connect(ui->triggerBox, SIGNAL(valueChanged(int)), ui->triggerSlider, SLOT(setValue(int)));
    connect(ui->triggerSlider, SIGNAL(valueChanged(int)), ui->triggerBox, SLOT(setValue(int)));

    connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(on_refreshTrackingButton_clicked()));
}

configWindow::~configWindow()
{
    delete ui;
}

QMap<QString, QVariant> configWindow::getSettings()
{
    QMap<QString, QVariant> settings;

    settings.insert("version", "0.1");

    QTime time = ui->lengthEdit->time();
    settings.insert("experiment/duration", time.hour()*60*60+time.minute()*60+time.second());
    settings.insert("experiment/stopAfterTimeout", ui->timeoutStopBox->isChecked());

    settings.insert("system/defaultDirectory", ui->directoryEdit->text());
    settings.insert("system/defaultFilename", ui->filenameEdit->text());
    settings.insert("system/updateInterval", ui->updateBox->value());


    settings.insert("video/device", ui->deviceBox->value());

    settings.insert("tracking/threshold", ui->threshSpin->value());
    settings.insert("tracking/maxArea", ui->maxAreaBox->value());
    settings.insert("tracking/minArea", ui->minAreaBox->value());

    settings.insert("mask/type", ui->typeBox->currentIndex());

    settings.insert("mask/X", ui->maskXBox->value());
    settings.insert("mask/Y", ui->maskYBox->value());
    settings.insert("mask/radius", ui->maskRadiusBox->value());

    settings.insert("tracking/minRat", ui->ratMinSize->value());
    settings.insert("tracking/maxRat", ui->ratMaxSize->value());
    settings.insert("tracking/minRobot", ui->robotMinSize->value());
    settings.insert("tracking/maxRobot", ui->robotMaxSize->value());

    settings.insert("shock/EntranceLatency", ui->entryBox->value());
    settings.insert("shock/InterShockLatency", ui->interBox->value());
    settings.insert("shock/ShockDuration", ui->durationBox->value());
    settings.insert("shock/OutsideRefractory", ui->refractoryBox->value());
    settings.insert("shock/triggerDistance", ui->triggerBox->value());

    return settings;
}


void configWindow::setSettings(QMap<QString, QVariant> settings){
    int length = settings.value("experiment/duration").toInt();
    int s = length%60;
    length /= 60;
    int m = length % 60;
    length /= 60;
    int h = length % 24;

    ui->lengthEdit->setTime(QTime(h, m, s));
    ui->timeoutStopBox->setChecked(settings.value("experiment/stopAfterTimeout").toBool());

    ui->deviceBox->setValue(settings.value("video/device").toInt());
    ui->threshSpin->setValue(settings.value("tracking/threshold").toInt());

    ui->typeBox->setCurrentIndex(settings.value("mask/type").toInt());

    ui->maskXBox->setValue(settings.value("mask/X").toDouble());
    ui->maskYBox->setValue(settings.value("mask/Y").toDouble());
    ui->maskRadiusBox->setValue(settings.value("mask/radius").toInt());

    ui->maxAreaBox->setValue(settings.value("tracking/maxArea").toDouble());
    ui->minAreaBox->setValue(settings.value("tracking/minArea").toDouble());

    ui->ratMinSize->setValue(settings.value("tracking/minRat").toDouble());
    ui->ratMaxSize->setValue(settings.value("tracking/maxRat").toDouble());
    ui->robotMinSize->setValue(settings.value("tracking/minRobot").toDouble());
    ui->robotMaxSize->setValue(settings.value("tracking/maxRobot").toDouble());

    ui->entryBox->setValue(settings.value("shock/EntranceLatency").toInt());
    ui->interBox->setValue(settings.value("shock/InterShockLatency").toInt());
    ui->durationBox->setValue(settings.value("shock/ShockDuration").toInt());
    ui->refractoryBox->setValue(settings.value("shock/OutsideRefractory").toInt());
    ui->triggerBox->setValue(settings.value("shock/triggerDistance").toInt());

    ui->directoryEdit->setText(settings.value("system/defaultDirectory").toString());
    ui->filenameEdit->setText(settings.value("system/defaultFilename").toString());

    ui->updateBox->setValue(settings.value("system/updateInterval").toInt());
}

void configWindow::on_testButton_clicked()
{
    capture.open(ui->deviceBox->value());

    Mat frame;
    capture >> frame;

    if (!frame.empty()){
        capturedFrame = QPixmap::fromImage(QImage((uchar*) frame.data,
                                                  frame.cols,
                                                  frame.rows,
                                                  frame.step,
                                                  QImage::Format_RGB888));
        valueMaskChanged();
    } else {
        QMessageBox result;
        result.setText("Error! Couldn't retrieve frame.");
        result.exec();
    }

    capture.release();
}

void configWindow::valueMaskChanged(){
    if (capturedFrame.height() > 0){
        QPoint center = QPoint(ui->maskXBox->value(), ui->maskYBox->value());

        QPixmap pixmap = QPixmap(capturedFrame);
        QPainter painter(&pixmap);
        painter.setPen(Qt::magenta);
        painter.drawLine(QPoint(center.x()-5, center.y()-5), QPoint(center.x()+5, center.y()+5));
        painter.drawLine(QPoint(center.x()-5, center.y()+5), QPoint(center.x()+5, center.y()-5));
        painter.drawEllipse(center, (int) ui->maskRadiusBox->value(), (int) ui->maskRadiusBox->value());
        painter.end();

        ui->videoLabel->setPixmap(pixmap);
        resize(minimumSizeHint());
    }
}

void configWindow::on_refreshTrackingButton_clicked()
{
    Mat frame;
    if (capture.isOpened()){
        capture >> frame;
    } else {
        capture.open(ui->deviceBox->value());
        capture >> frame;
        capture.release();
    }

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

void configWindow::on_browseButton_clicked()
{
    QString directoryPath = QFileDialog::getExistingDirectory(this);
    if (!directoryPath.isEmpty()){
        ui->directoryEdit->setText(directoryPath);
    }
}

void configWindow::on_checkBox_stateChanged(int state)
{
    if (state == Qt::Checked){
        ui->refreshTrackingButton->setDisabled(true);
        capture.open(ui->deviceBox->value());
        refreshTimer.start(100);
    } else {
        ui->refreshTrackingButton->setDisabled(false);
        refreshTimer.stop();
        capture.release();
    }
}
