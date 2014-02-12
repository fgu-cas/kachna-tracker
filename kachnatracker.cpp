#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "kachnatracker.h"
#include "ui_kachnatracker.h"
#include "detectordialog.h"
#include <iostream>
#include <QFileDialog>
#include <QTimer>

using namespace cv;

kachnatracker::kachnatracker(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::kachnatracker)
{
    ui->setupUi(this);
    image_label = ui->imageLabel;
    detectorDialog = new DetectorDialog(this);


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
    Mat cap_frame;
    capture >> cap_frame;

    Mat pre_result;
    if (settings.blur == 0){
        cap_frame.convertTo(pre_result, -1, settings.alpha, settings.beta);
    } else {
        Mat blurred;
        blur(cap_frame, blurred, Size(settings.blur, settings.blur));
        blurred.convertTo(pre_result, -1, settings.alpha, settings.beta);
    }

    Mat result;
    threshold(pre_result, result, settings.threshold, 255, THRESH_TOZERO);


    std::vector<KeyPoint> keypoints_1;
    detector.detect(result, keypoints_1);

    Mat frame;
    drawKeypoints(result, keypoints_1, frame, Scalar::all(-1), DrawMatchesFlags::DEFAULT );

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
