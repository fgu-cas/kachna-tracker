#ifndef DETECTORCOLOR_H
#define DETECTORCOLOR_H

#include "detector.h"

#include <memory>

#include <QMap>
#include <QVariant>
#include <QElapsedTimer>

#include <opencv2/core/core.hpp>


struct pointRange {
	int hue = 0;
	double minimum_size = -1;
	double maximum_size = -1;
};

using namespace cv;

class DetectorColor : public Detector
{
public:
	DetectorColor(const QMap<QString, QVariant> &settings, int h, int w);

	Mat process(Mat *frame);
	Mat analyze(Mat *frame);
	std::vector<DetectedPoint> detect(Mat *frame);
	pointPair find(Mat *frame);
private:
	std::unique_ptr<SimpleBlobDetector> detector;

	bool bothPoints = false;

	double min, max;
	int hue_tol, val, sat;

	pointRange ratFront;
	pointRange ratBack;
	pointRange robotFront;
	pointRange robotBack;
};

#endif // DETECTORCOLOR_H
