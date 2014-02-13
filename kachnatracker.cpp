#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "kachnatracker.h"
#include "ui_kachnatracker.h"
#include "detectordialog.h"
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <cmath>

using namespace cv;

kachnatracker::kachnatracker(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::kachnatracker)
{
    ui->setupUi(this);
    ui->distanceLabel->setTextFormat(Qt::RichText);
    ui->imageLabel->setScaledContents(true);
    ui->originalLabel->setScaledContents(true);
    detectorDialog = new DetectorDialog(this);
    badFrames = 0;
    counter = 0;

    outputMat = new Mat();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateFrame()));

    milliseconds = 0;
}

kachnatracker::~kachnatracker()
{
    delete ui;
}

void kachnatracker::updateSettings(){
    settings = detectorDialog->getSettings();

    cv::SimpleBlobDetector::Params params;
    params.minDistBetweenBlobs = 10.0f;
    params.filterByInertia = false;
    params.filterByConvexity = false;
    params.filterByColor = true;
    params.blobColor = 255;
    params.filterByCircularity = false;
    params.filterByArea = true;
    params.minArea = settings.minSize;
    params.maxArea = settings.maxSize;

    detector = SimpleBlobDetector(params);
    this->updateFrame();
}

void kachnatracker::updateFrame(){
    Mat displayFrame;
    if (timer->isActive()){
        capture >> frame;
    }
    if (frame.empty()){
        if (timer->isActive()){
            this->timer->stop();
            QMessageBox alert;
            alert.setText("Bad frames: " + QString::number(this->badFrames));
            alert.exec();
        }
        return;
    }

    if (settings.blur == 0){
        frame.convertTo(displayFrame, -1, settings.alpha, settings.beta);
    } else {
        blur(frame, displayFrame, Size(settings.blur, settings.blur));
        displayFrame.convertTo(displayFrame, -1, settings.alpha, settings.beta);
    }

    threshold(displayFrame, displayFrame, settings.threshold, 255, THRESH_TOZERO);


    std::vector<KeyPoint> keypoints;
    detector.detect(displayFrame, keypoints);

    ui->pointsList->clear();
    for(std::vector<KeyPoint>::iterator it = keypoints.begin(); it != keypoints.end(); ++it) {
        std::stringstream ss;
        ss << "Point #" << (it - keypoints.begin());
        ss << ", X: " << (it->pt.x);
        ss << "Y: " << (it->pt.y);
        std::string tmp = std::string(ss.str());
        ui->pointsList->addItem(QString(tmp.data()));

        if (prevPoint.x){
            if (keypoints.size() == 2){
                counter++;
                if (counter < 37 ){
                    line(*outputMat, prevPoint, keypoints[1].pt, Scalar(0, 0, 255));
                } else {
                    circle(*outputMat, keypoints[1].pt, 4, Scalar(0, 0, 255), -1);
                    counter = 0;
                }
            }
        }
        if (keypoints.size() == 2){
            prevPoint = keypoints[1].pt;
        }
    }

    QString distanceString = "<html><head/><body><p align=\"center\"><span style=\"font-size:24pt; color:";
    if (keypoints.size() == 2){
        double distance = cv::norm(keypoints[0].pt-keypoints[1].pt);
        distanceString += "#00ff00;\">" + QString::number(distance);
    } else {
        distanceString += "#ff0000;\">BAD FRAME";
        this->badFrames++;
        if (settings.pause){
            timer->stop();
        }
    }
    distanceString += "</span></p></body></html>";
    ui->distanceLabel->setText(distanceString);

    drawKeypoints(frame, keypoints, displayFrame, Scalar(255, 0, 0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

    QImage qt_image = QImage((uchar*) displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_RGB888);
    ui->originalLabel->setPixmap(QPixmap::fromImage(qt_image));

    QImage qt_orig_image = QImage((uchar*) outputMat->data, outputMat->cols, outputMat->rows, outputMat->step, QImage::Format_RGB888);
    ui->imageLabel->setPixmap(QPixmap::fromImage(qt_orig_image));

    milliseconds += interval;
}

void kachnatracker::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Files (*.*)"));

    this->fileName = fileName;
    capture = VideoCapture(fileName.toStdString());
    if (capture.isOpened()){
        double fps = capture.get(CV_CAP_PROP_FPS);
        interval = 1000/fps;
        Mat tempFrame;
        capture >> tempFrame;
        outputMat->create(tempFrame.size(),tempFrame.type());
        outputMat->setTo(Scalar(255, 255, 255));
        updateSettings();
        timer->start(interval);
    }

}


void kachnatracker::on_pauseButton_clicked()
{
   if (timer->isActive()){
     timer->stop();
   } else {
       capture = VideoCapture(fileName.toStdString());
       if (capture.isOpened()){
           capture.set(CV_CAP_PROP_POS_MSEC, milliseconds);
           timer->start(interval);
       }
   }
}

void kachnatracker::on_settingsButton_clicked()
{
    detectorDialog->show();
}
