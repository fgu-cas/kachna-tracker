#include "detector_color.h"

#include <opencv2/imgproc/imgproc.hpp>


DetectorColor::DetectorColor(QMap<QString, QVariant> settings, int h, int w)
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
}

Mat DetectorColor::process(Mat *frame){
    Mat processedFrame;
    frame->copyTo(processedFrame, mask);
    cv::cvtColor(processedFrame, processedFrame, CV_BGR2RGB);
    return processedFrame;
}

Mat DetectorColor::filter(Mat *frame, colorRange range){
    int hue = range.hue;
    int tol = range.hue_tolerance;
    int sat = range.saturation_low;
    int val = range.value_low;

    Mat mask_result;

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
    /*

    if (!frame->empty() && frame->channels() == 3){
        Mat processedFrame = process(frame);

        Mat frame_hsv;
        cvtColor(frame, frame_hsv, CV_RGB2HSV);

        Mat mask;

        int hue = ui->hueBox->value();
        int range = ui->rangeBox->value();
        int sat_low = ui->saturationBox->value();
        int val_low = ui->valueBox->value();

        detector->detect(processedFrame, result);
    }

    return result;
}




        if (range > hue){
            Mat mask_a, mask_b;
            inRange(frame_hsv, Scalar(0, sat_low, val_low),
                               Scalar((hue+range)/2, 255, 255), mask_a);
            inRange(frame_hsv, Scalar((360-(range-hue))/2, sat_low, val_low),
                               Scalar(360/2, 255, 255), mask_b);
            add(mask_a, mask_b, mask);
        } else if (hue + range > 360){
            Mat mask_a, mask_b;
            inRange(frame_hsv, Scalar((hue-range)/2, sat_low, val_low),
                               Scalar(360/2, 255, 255), mask_a);
            inRange(frame_hsv, Scalar(0, sat_low, val_low),
                               Scalar((range-(360-hue))/2, 255, 255), mask_b);
            add(mask_a, mask_b, mask);
        } else {
            inRange(frame_hsv, Scalar((hue-range)/2, sat_low, val_low),
                               Scalar((hue+range)/2, 255, 255), mask);
        }

        frame.copyTo(displayFrame, mask);
    } else {
        displayFrame = frame;
    }

    QPixmap pixmap = QPixmap::fromImage(QImage((uchar*) displayFrame.data,
                                               displayFrame.cols,
                                               displayFrame.rows,
                                               displayFrame.step,
                                               QImage::Format_RGB888));


    ui->label->setPixmap(pixmap);
    */
    return result;
}
