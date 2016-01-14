#include "detector_color.h"
#include <opencv2/imgproc/imgproc.hpp>

#include <QtMath>
#include <math.h>


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

    // to be replaced w/ a more sensible approach
    ratFront.hue = settings.value("tracking/color/ratFront/hue").toInt();
    ratFront.hue_tolerance = settings.value("tracking/color/ratFront/hue_tolerance").toInt();
    ratFront.saturation_low = settings.value("tracking/color/ratFront/saturation_low").toInt();
    ratFront.value_low = settings.value("tracking/color/ratFront/value_low").toInt();

    ratBack.hue = settings.value("tracking/color/ratBack/hue").toInt();
    ratBack.hue_tolerance = settings.value("tracking/color/ratBack/hue_tolerance").toInt();
    ratBack.saturation_low = settings.value("tracking/color/ratBack/saturation_low").toInt();
    ratBack.value_low = settings.value("tracking/color/ratBack/value_low").toInt();

    robotFront.hue = settings.value("tracking/color/robotFront/hue").toInt();
    robotFront.hue_tolerance = settings.value("tracking/color/robotFront/hue_tolerance").toInt();
    robotFront.saturation_low = settings.value("tracking/color/robotFront/saturation_low").toInt();
    robotFront.value_low = settings.value("tracking/color/robotFront/value_low").toInt();

    robotBack.hue = settings.value("tracking/color/robotBack/hue").toInt();
    robotBack.hue_tolerance = settings.value("tracking/color/robotBack/hue_tolerance").toInt();
    robotBack.saturation_low = settings.value("tracking/color/robotBack/saturation_low").toInt();
    robotBack.value_low = settings.value("tracking/color/robotBack/value_low").toInt();
}

Mat DetectorColor::process(Mat *frame){
    Mat result;
    frame->copyTo(result, mask);
    cv::cvtColor(result, result, CV_BGR2HSV);
    return result;
}

Mat DetectorColor::filter(Mat *frame, colorRange range){
    Mat mask_result;
    int hue = range.hue;
    int tol = range.hue_tolerance;
    int sat = range.saturation_low;
    int val = range.value_low;

    if (tol > hue){
        Mat mask_a, mask_b;
        inRange(*frame, Scalar(0, sat, val),
                           Scalar((hue+tol)/2, 255, 255), mask_a);
        inRange(*frame, Scalar((360-(tol-hue))/2, sat, val),
                           Scalar(360/2, 255, 255), mask_b);
        add(mask_a, mask_b, mask_result);
    } else if (hue + tol > 360){
        Mat mask_a, mask_b;
        inRange(*frame, Scalar((hue-tol)/2, sat, val),
                           Scalar(360/2, 255, 255), mask_a);
        inRange(*frame, Scalar(0, sat, val),
                           Scalar((tol-(360-hue))/2, 255, 255), mask_b);
        add(mask_a, mask_b, mask_result);
    } else {
        inRange(*frame, Scalar((hue-tol)/2, sat, val),
                           Scalar((hue+tol)/2, 255, 255), mask_result);
    }

    Mat result;
    frame->copyTo(result, mask_result);
    return result;
}

std::vector<KeyPoint> DetectorColor::detect(Mat *frame){
    std::vector<KeyPoint> result;
    std::vector<KeyPoint> tmp;

    if (!frame->empty()){
        Mat workFrame = process(frame);

        Mat rat_front = filter(&workFrame, ratFront);
        detector->detect(rat_front, tmp);
        for (KeyPoint point : tmp){
            point.class_id = RAT_FRONT;
            result.push_back(point);
        }

        Mat rat_back = filter(&workFrame, ratBack);
        detector->detect(rat_back, tmp);
        for (KeyPoint point : tmp){
            point.class_id = RAT_BACK;
            result.push_back(point);
        }

        Mat robot_front = filter(&workFrame, robotFront);
        detector->detect(robot_front, tmp);
        for (KeyPoint point : tmp){
            point.class_id = ROBOT_FRONT;
            result.push_back(point);
        }

        Mat robot_back = filter(&workFrame, robotBack);
        detector->detect(robot_back, tmp);
        for (KeyPoint point : tmp){
            point.class_id = ROBOT_BACK;
            result.push_back(point);
        }
    }

    return result;
}

Detector::keypointPair DetectorColor::find(Mat *frame){
    keypointPair result;

    std::vector<KeyPoint> points = detect(frame);

    KeyPoint rat_front;
    KeyPoint rat_back;
    KeyPoint robot_front;
    KeyPoint robot_back;

    for (KeyPoint point : points){
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
    }

    if (robot_front.size > 0 && robot_back.size > 0){
        result.robot.pt.x = (robot_front.pt.x + robot_back.pt.x) / 2;
        result.robot.pt.y = (robot_front.pt.y + robot_back.pt.y) / 2;
        result.robot.angle = qRadiansToDegrees(qAtan2(robot_back.pt.y - robot_front.pt.y,
                                   robot_back.pt.x - robot_front.pt.x));
        result.robot.angle = fmod(result.robot.angle + 270, 360);
    }

    return result;
}
