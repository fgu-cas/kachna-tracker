#include "detector_threshold.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <qmath.h>
#include <stdexcept>


DetectorThreshold::DetectorThreshold(const QMap<QString, QVariant> &settings, int h, int w)
    : Detector(settings, h, w) {
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

    img_threshold = settings.value("tracking/threshold/threshold").toInt();

    detector.reset(new SimpleBlobDetector(params));

    maxRat = settings.value("tracking/threshold/maxRat").toDouble();
    minRat = settings.value("tracking/threshold/minRat").toDouble();
    maxRobot = settings.value("tracking/threshold/maxRobot").toDouble();
    minRobot = settings.value("tracking/threshold/minRobot").toDouble();

    // multiple_reaction = settings->value("faults/multipleReaction").toInt();
    skip_reaction = settings.value("faults/skipReaction").toInt();
    skip_distance = settings.value("faults/skipDistance").toInt();
    skip_timeout = settings.value("faults/skipTimeout").toInt();
}

Mat DetectorThreshold::process(Mat *frame){
    Mat processedFrame;
    frame->copyTo(processedFrame, mask);
    cv::cvtColor(processedFrame, processedFrame, CV_BGR2GRAY);
    return processedFrame;
}

Mat DetectorThreshold::analyze(Mat *frame){
    Mat result;
    threshold(*frame, result, img_threshold, 255, THRESH_BINARY);
    return result;
}

std::vector<KeyPoint> DetectorThreshold::detect(Mat *frame){
    std::vector<KeyPoint> result;

    if (!frame->empty() && frame->channels() == 3){
        Mat workFrame = process(frame);
        workFrame = analyze(&workFrame);
        detector->detect(workFrame, result);
    }

    return result;
}

Detector::keypointPair DetectorThreshold::find(Mat *frame){
    keypointPair result;

    std::vector<KeyPoint> keypoints = detect(frame);

    for (unsigned i = 0; i<keypoints.size(); i++){
       KeyPoint keypoint = keypoints[i];
       if (keypoint.size > minRat && keypoint.size < maxRat){
           if (skip_reaction == 1){
               if (lastPoints.rat.size == 0 ||
                   getDistance(lastPoints.rat, keypoint) < skip_distance ||
                   ratTimer.elapsed() > skip_timeout){
                       result.rat = keypoint;
                       lastPoints.rat = keypoint;
                       ratTimer.start();
                }
           } else {
              result.rat = keypoint;
           }
       }

       if (keypoint.size > minRobot && keypoint.size < maxRobot){
           if (skip_reaction == 1){
               if (lastPoints.robot.size == 0 ||
                   getDistance(lastPoints.robot, keypoint) < skip_distance ||
                   robotTimer.elapsed() > skip_timeout){
                       result.robot = keypoint;
                       lastPoints.robot = keypoint;
                       robotTimer.start();
               }
           } else {
              result.robot = keypoint;
           }
       }
    }

    return result;
}

double DetectorThreshold::getDistance(KeyPoint a, KeyPoint b){
    double dx = a.pt.x - b.pt.x;
    double dy = a.pt.y - b.pt.y;
    return qSqrt(dx*dx + dy*dy);
}

