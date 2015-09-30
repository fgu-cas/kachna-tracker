#include "detector.h"
#include <opencv2/imgproc/imgproc.hpp>

#include <stdexcept>

Detector::Detector(QMap<QString, QVariant> settings, int h, int w){
    cv::SimpleBlobDetector::Params params;
    params.minDistBetweenBlobs = 10.0f;
    params.filterByInertia = false;
    params.filterByConvexity = false;
    params.filterByColor = true;
    params.blobColor = 255;
    params.filterByCircularity = false;
    params.filterByArea = true;
    params.minArea = 20;
    params.maxArea = 700;

    img_threshold = settings.value("tracking/threshold").toInt();

    detector.reset(new SimpleBlobDetector(params));

    mask = Mat(h, w, CV_8UC1);
    mask.setTo(Scalar(0));
    circle(mask, Point2f(settings.value("arena/X").toInt(), settings.value("arena/Y").toInt()),
           settings.value("arena/radius").toInt(), Scalar(255), -1);
}

Mat Detector::process(Mat *frame){
    Mat processedFrame;
    frame->copyTo(processedFrame, mask);
    cv::cvtColor(processedFrame, processedFrame, CV_RGB2GRAY);
    threshold(processedFrame, processedFrame, img_threshold, 255, THRESH_BINARY);
    return processedFrame;
}

std::vector<KeyPoint> Detector::detect(Mat *frame){
    std::vector<KeyPoint> result;

    if (!frame->empty() && frame->channels() == 3){
        Mat processedFrame = process(frame);
        detector->detect(processedFrame, result);
    }

    return result;
}
