#ifndef DETECTOR_H
#define DETECTOR_H

#include <memory>

#include <QMap>
#include <QVariant>
#include <QElapsedTimer>

#include "params.h"

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

using namespace cv;

class Detector
{
public:
	Detector(const QMap<QString, QVariant> &settings, int h, int w);

	struct pointPair { DetectedPoint rat; DetectedPoint robot; };

	virtual Mat process(Mat *frame) = 0;
	virtual Mat analyze(Mat *frame) = 0;
	virtual pointPair find(Mat *frame) = 0;
	virtual std::vector<DetectedPoint> detect(Mat *frame) = 0;

protected:
	Mat mask;
	cv::Rect maskRect;
};




#endif // DETECTOR_H
