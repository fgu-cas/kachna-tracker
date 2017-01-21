#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QElapsedTimer>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "params.h"
#include "mainwindow.h"
#include "detector_threshold.h"
#include "Logger.h"
#include "experimentlogger.h"
#include "abstracthardware.h"

using namespace cv;

class Experiment : public QObject
{
	Q_OBJECT
public:
	explicit Experiment(QMap<QString, QVariant> *settings, Logger*, QObject *parent = 0);
	~Experiment();
	bool isRunning();
	QString getLog();

signals:
	void update(ExperimentState state);
	void finished();

private:
	int arenaPWM = 0;
	int arenaDirection = 0;

	// System
	ExperimentStats stats;
	std::unique_ptr<ExperimentLogger> experimentLogger;
	Logger* logger;

	QTimer timer;
	QElapsedTimer elapsedTimer;
	qint64 finishedTime;

	Detector::pointPair lastKeypoints;

	bool ratRobot;

	bool shockIsElectric;
	bool shockOut;
	bool synchOut;
	bool synchInv;

	// Capture
	VideoCapture capture;
	bool isLive;
	Mat lastFrame;

	// Detection
	std::unique_ptr<Detector> detector;

	int multiple_reaction;
	int skip_reaction;
	int skip_distance;
	int skip_timeout;
	Detector::pointPair lastPoints;
	QElapsedTimer ratTimer;
	QElapsedTimer robotTimer;

	// Shock
	enum shockStates {
		OUTSIDE,
		DELAYING,
		SHOCKING,
		PAUSE,
		REFRACTORY
	} shockState;
	double shockLevel;
	int currentShockLevel = 2;
	void outputShock(int level);

	Shock shock;

	qint64 lastChange;
	qint64 lastLight = 0;

	Arena arena;

	QList<Area> areas;
	QList<Counter> counters;
	QList<Action> actions;

	enum TriggerState {
		OFF,
		RISING,
		ON
	};

	QMap<QString, TriggerState> triggerStates;

	QString hardwareDevice;
	std::unique_ptr<AbstractHardware> hardware;

	public slots:
	bool start();
	void stop();
	void setShockLevel(int level);

	private slots:
	void processFrame();

};

#endif // EXPERIMENT_H
