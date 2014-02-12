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
    image_label = ui->imageLabel;
    detectorDialog = new DetectorDialog(this);
    badFrames = 0;


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
    params.minDistBetweenBlobs = 50.0f;
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
    Mat frame;
    capture >> frame;

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
        frame.convertTo(frame, -1, settings.alpha, settings.beta);
    } else {
        blur(frame, frame, Size(settings.blur, settings.blur));
        frame.convertTo(frame, -1, settings.alpha, settings.beta);
    }

    threshold(frame, frame, settings.threshold, 255, THRESH_TOZERO);


    std::vector<KeyPoint> keypoints;
    detector.detect(frame, keypoints);

    ui->pointsList->clear();
    for(std::vector<KeyPoint>::iterator it = keypoints.begin(); it != keypoints.end(); ++it) {
        std::stringstream ss;
        ss << "Point #" << (it - keypoints.begin());
        ss << ", X: " << (it->pt.x);
        ss << "Y: " << (it->pt.y);
        std::string tmp = std::string(ss.str());
        ui->pointsList->addItem(QString(tmp.data()));
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

    drawKeypoints(frame, keypoints, frame, Scalar(255, 0, 0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

    QImage qt_image = QImage((uchar*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    this->image_label->setPixmap(QPixmap::fromImage(qt_image));

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
