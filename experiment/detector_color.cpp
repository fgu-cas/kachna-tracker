#include "detector_color.h"
#include <opencv2/imgproc/imgproc.hpp>

#include <QtMath>
#include <math.h>

#include <QDebug>

DetectorColor::DetectorColor(const QMap<QString, QVariant> &settings, int h, int w)
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
    detector.reset(new SimpleBlobDetector(params));

    if (maskRect.x + maskRect.width > w) maskRect.width = w - maskRect.x;
    if (maskRect.y + maskRect.height > h) maskRect.height = h - maskRect.y;

    min = settings.value("tracking/color/global_minimum_size").toDouble();
    max = settings.value("tracking/color/global_maximum_size").toDouble();

    hue_tol = settings.value("tracking/color/hue_tolerance").toInt();
    sat = settings.value("tracking/color/saturation_threshold").toInt();
    val = settings.value("tracking/color/value_threshold").toInt();

    // to be replaced w/ a more sensible approach
    ratFront.hue = settings.value("tracking/color/ratFront/hue").toInt();
    ratFront.minimum_size = settings.value("tracking/color/ratFront/minimum_size").toInt();
    ratFront.maximum_size = settings.value("tracking/color/ratFront/maximum_size").toInt();

    ratBack.hue = settings.value("tracking/color/ratBack/hue").toInt();
    ratBack.minimum_size = settings.value("tracking/color/ratBack/minimum_size").toInt();
    ratBack.maximum_size = settings.value("tracking/color/ratBack/maximum_size").toInt();

    robotFront.hue = settings.value("tracking/color/robotFront/hue").toInt();
    robotFront.minimum_size = settings.value("tracking/color/robotFront/minimum_size").toInt();
    robotFront.maximum_size = settings.value("tracking/color/robotFront/maximum_size").toInt();

    robotBack.hue = settings.value("tracking/color/robotBack/hue").toInt();
    robotBack.minimum_size = settings.value("tracking/color/robotBack/minimum_size").toInt();
    robotBack.maximum_size = settings.value("tracking/color/robotBack/maximum_size").toInt();
}

Mat DetectorColor::process(Mat *frame){
    Mat result;
    frame->copyTo(result, mask);
    result = result(maskRect);
    cv::cvtColor(result, result, CV_BGR2HSV);
    return result;
}

Mat DetectorColor::analyze(Mat *frame){
    Mat threshMask;
    inRange(*frame, Scalar(0, sat, val),
                       Scalar(255, 255, 255), threshMask);
    return threshMask;
}

std::vector<Detector::Point> DetectorColor::detect(Mat *frame){
    std::vector<Point> result;

    if (!frame->empty()){
        Mat HSVFrame = process(frame);
        Mat binarizedFrame = analyze(&HSVFrame);

        std::vector< std::vector<Point> > contours;
        findContours(binarizedFrame, contours, RETR_LIST, CHAIN_APPROX_NONE);

        for (uint i = 0; i < contours.size();  i++){
            Moments moms = moments(Mat(contours[i]));
            if (moms.m00 == 0) continue;
            Detector::Point point;
            point.pt.x = (moms.m10 / moms.m00) + maskRect.x;
            point.pt.y = (moms.m01 / moms.m00) + maskRect.y;
            point.size = std::sqrt(moms.m00/CV_PI);
            if (point.size > min && point.size < max){
                Rect bounding = boundingRect(contours[i]);
                Mat cropped_hsv = HSVFrame(bounding);
                Mat cropped_mask = binarizedFrame(bounding);
                Mat tmp_mat;
                cropped_hsv.copyTo(tmp_mat, cropped_mask);
                uint8_t* pixels = (uint8_t*)tmp_mat.data;
                double sum_cos = 0;
                double sum_sin = 0;
                int count = 0;
                for (int i = 0; i < tmp_mat.rows; i++){
                    for (int j = 0; j < tmp_mat.cols; j++){
                        int h = pixels[i*tmp_mat.cols*3 + j*3 + 0];
                        int s = pixels[i*tmp_mat.cols*3 + j*3 + 1];
                        int v = pixels[i*tmp_mat.cols*3 + j*3 + 2];
                        if (s >= sat && v >= val){
                            count++;
                            sum_cos += cos((h*2)*CV_PI/180);
                            sum_sin += sin((h*2)*CV_PI/180);
                        }
                    }
                }
                if (count == 0){
                    point.class_id = -1;
                    result.push_back(point);
                } else {
                    int average = atan2(sum_sin/count, sum_cos/count)*180/CV_PI;
                    if (average < 0) average = 360+average;
                    int distance;

                    distance = abs(ratFront.hue-average);
                    if (distance > 180) distance = 360 - distance;
                    if (distance < hue_tol){
                        point.class_id = RAT_FRONT;
                        result.push_back(point);
                        continue;
                    }
                    distance = abs(ratBack.hue-average);
                    if (distance > 180) distance = 360 - distance;
                    if (distance < hue_tol){
                        point.class_id = RAT_BACK;
                        result.push_back(point);
                        continue;
                    }
                    distance = abs(robotFront.hue-average);
                    if (distance > 180) distance = 360 - distance;
                    if (distance < hue_tol){
                        point.class_id = ROBOT_FRONT;
                        result.push_back(point);
                        continue;
                    }
                    distance = abs(robotBack.hue-average);
                    if (distance > 180) distance = 360 - distance;
                    if (distance < hue_tol){
                        point.class_id = ROBOT_BACK;
                        result.push_back(point);
                        continue;
                    }

                    point.class_id = -1;
                    result.push_back(point);
                }
            }
        }
    }

    return result;
}

Detector::pointPair DetectorColor::find(Mat *frame){
    pointPair result;

    std::vector<Detector::Point> points = detect(frame);

    Detector::Point rat_front;
    Detector::Point rat_back;
    Detector::Point robot_front;
    Detector::Point robot_back;

    for (Detector::Point point : points){
        switch (point.class_id){
            case RAT_FRONT:
                rat_front = point;
                break;
            case RAT_BACK:
                rat_back = point;
                break;
           case ROBOT_FRONT:
                robot_front = point;
                break;
           case ROBOT_BACK:
                robot_back = point;
                break;
           default:
                break;
        }
    }

    if (rat_front.size > 0 && rat_back.size > 0){
        result.rat.pt.x = (rat_front.pt.x + rat_back.pt.x) / 2;
        result.rat.pt.y = (rat_front.pt.y + rat_back.pt.y) / 2;
        result.rat.angle = qRadiansToDegrees(qAtan2(rat_back.pt.y - rat_front.pt.y,
                                             rat_back.pt.x - rat_front.pt.x));
        result.rat.angle = fmod(result.rat.angle + 270, 360);
        result.rat.size = 1;
    }

    if (robot_front.size > 0 && robot_back.size > 0){
        result.robot.pt.x = (robot_front.pt.x + robot_back.pt.x) / 2;
        result.robot.pt.y = (robot_front.pt.y + robot_back.pt.y) / 2;
        result.robot.angle = qRadiansToDegrees(qAtan2(robot_back.pt.y - robot_front.pt.y,
                                   robot_back.pt.x - robot_front.pt.x));
        result.robot.angle = fmod(result.robot.angle + 270, 360);
        result.robot.size = 1;
    }

    return result;
}
