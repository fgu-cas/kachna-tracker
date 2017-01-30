#include "experiment.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QDateTime>
#include "arenomat.h"
#include "dummyhardware.h"
#include "dio24.h"

#define LIGHT_LIMIT 500

Experiment::Experiment(QMap<QString, QVariant> *settings, Logger* logger, QObject *parent) :
	QObject(parent), logger(logger) {
	int deviceIndex = settings->value("video/device").toInt();
	if (deviceIndex == -1) {
		capture.open(settings->value("video/filename").toString().toStdString());
		isLive = false;
	}
	else {
		capture.open(deviceIndex);
		isLive = true;
	}

	if (settings->value("tracking/type").toInt() == 0) {
		detector.reset(new DetectorThreshold(*settings, capture.get(CV_CAP_PROP_FRAME_HEIGHT), capture.get(CV_CAP_PROP_FRAME_WIDTH)));
	}
	else {
		detector.reset(new DetectorColor(*settings, capture.get(CV_CAP_PROP_FRAME_HEIGHT), capture.get(CV_CAP_PROP_FRAME_WIDTH)));
	}

	timer.setInterval(40);
	timer.setTimerType(Qt::PreciseTimer);
	connect(&timer, SIGNAL(timeout()), this, SLOT(processFrame()));

	ratRobot = settings->value("experiment/mode").toInt() == 0 ? false : true;

	synchOut = settings->value("output/sync_enabled").toBool();
	synchInv = settings->value("output/sync_inverted").toBool();
	shockOut = settings->value("output/shock").toBool();

	// multiple_reaction = settings->value("faults/multipleReaction").toInt();
	skip_reaction = settings->value("faults/skipReaction").toInt();
	skip_distance = settings->value("faults/skipDistance").toInt();
	skip_timeout = settings->value("faults/skipTimeout").toInt();

	arena.x = settings->value("arena/X").toInt();
	arena.y = settings->value("arena/Y").toInt();
	arena.size = settings->value("arena/size").toDouble();
	arena.radius = settings->value("arena/radius").toInt();

	if (settings->value("shock/mode").toInt() == 0) {
		shockIsElectric = true;
		shockLevel = settings->value("shock/initialShock").toInt();
	}
	else {
		shockIsElectric = false;
		shockLevel = 1;
	}
	shock.delay = settings->value("shock/EntranceLatency").toInt();
	shock.in_delay = settings->value("shock/InterShockLatency").toInt();
	shock.length = settings->value("shock/ShockDuration").toInt();
	shock.refractory = settings->value("shock/OutsideRefractory").toInt();

	arenaPWM = settings->value("arena/PWM").toInt();

	QString direction = settings->value("arena/direction").toString();
	if (direction == "CW") {
		arenaDirection = 1;
	}
	else if (direction == "CCW") {
		arenaDirection = 2;
	}
	else {
		arenaDirection = 0;
	}

	counters = settings->value("actions/counters").value<QList<Counter>>();
	areas = settings->value("actions/areas").value<QList<Area>>();
	actions = settings->value("actions/actions").value<QList<Action>>();

	shockState = OUTSIDE;

	stats.goodFrames = 0;
	stats.badFrames = 0;
	stats.entryCount = 0;
	stats.shockCount = 0;
	stats.initialShock = 0;
	lastChange = 0;

	experimentLogger.reset(new ExperimentLogger(shock, arena, areas, counters, actions));
	connect(this, SIGNAL(update(ExperimentState)), experimentLogger.get(), SLOT(add(ExperimentState)));

	hardwareDevice = settings->value("hardware/device").toString();

	for (Counter counter : counters) {
		triggerStates[counter.id] = TriggerState::OFF;
	}

	for (Area area : areas) {
		triggerStates[area.id] = TriggerState::OFF;
	}
}

Experiment::~Experiment() {
	hardware.reset();
}

bool Experiment::start() {
	timer.start();
	elapsedTimer.start();
	experimentLogger->setStart(QDateTime::currentMSecsSinceEpoch());

	if (isLive) {
		if (hardwareDevice.startsWith("PCI-DIO24")) {
			hardware.reset(new DIO24(logger));
		}
		else {
			hardware.reset(new Arenomat(logger, hardwareDevice));
		}
	}
	else {
		hardware.reset(new DummyHardware(logger));
	}

	if (!hardware->check()) {
		logger->log("Hardware check failed", Logger::ERROR);
		return false;
	}

	if (isLive && arenaDirection > 0) {
		Arenomat* mat = dynamic_cast<Arenomat*>(hardware.get());
		mat->setTurntableDirection(arenaDirection);
		mat->setTurntablePWM(arenaPWM);
	}
	logger->log("Experiment started", Logger::INFO);
	return true;
}

void Experiment::stop() {
	hardware->setShock(0);
	hardware->shutdown();

	this->timer.stop();
	capture.release();
	finishedTime = elapsedTimer.elapsed();

	logger->log("Experiment stopped", Logger::INFO);

	emit finished();
}

bool Experiment::isRunning() {
	return this->timer.isActive();
}

void Experiment::processFrame() {
	// Output sync signal
	if (synchOut) hardware->setSync(!synchInv);

	// Process counters
	for (int i = 0; i < counters.size(); i++) {
		Counter& counter = counters[i];

		if (counter.active && (elapsedTimer.elapsed() - counter.lastUpdated) > counter.period * 1000) {
			counter.lastUpdated = elapsedTimer.elapsed();
			counter.value++;
		}
		if (counter.value > counter.limit) {
			counter.value = 0;
			if (counter.singleShot) {
				counter.active = false;
				logger->log(QString("Counter \"%1\" hit limit, resetting and stopping").arg(counter.id));
			}
			else {
				logger->log(QString("Counter \"%1\" hit limit, resetting").arg(counter.id));
			}
		}
	}


	//Process the frame
	Mat frame;
	bool read = capture.read(frame);
	if (!read && !isLive) { //We've reached the end of the file
		kachnatracker* mainwindow = dynamic_cast<kachnatracker*>(parent());
		mainwindow->experimentTimeout();
		return;
	}
	qint64 timestamp = elapsedTimer.elapsed();
	lastFrame = frame;

	// Detect points
	Detector::pointPair points = detector->find(&frame);
	if (!ratRobot) points.robot = DetectedPoint();
	bool badFrame = points.rat.size == -1 || (ratRobot && points.robot.size == -1);
	if (skip_reaction == 1) {
		if (lastPoints.rat.size == -1 ||
			cv::norm(lastPoints.rat.pt - points.rat.pt) < skip_distance ||
			ratTimer.elapsed() > skip_timeout) {
			lastPoints.rat = points.rat;
			ratTimer.start();
		}
		else {
			points.rat = DetectedPoint();
		}

		if (lastPoints.robot.size == -1 ||
			cv::norm(lastPoints.robot.pt - points.robot.pt) < skip_distance ||
			robotTimer.elapsed() > skip_timeout) {
			lastPoints.robot = points.robot;
			robotTimer.start();
		}
		else {
			points.robot = DetectedPoint();
		}
	}
	lastKeypoints = points;

	// Find active triggers
	QStringList activeTriggers;
	bool found;
	if (badFrame) {
		stats.badFrames++;

		for (int i = 0; i < counters.size(); i++) {
			Counter counter = counters.at(i);
			if (counter.value >= counter.limit) {
				activeTriggers.append(counter.id);
			}
		}
	}
	else {
		stats.goodFrames++;

		for (int i = 0; i < counters.size(); i++) {
			Counter counter = counters.at(i);
			if (counter.value >= counter.limit) {
				activeTriggers.append(counter.id);
				if (triggerStates[counter.id] == TriggerState::OFF) {
					triggerStates[counter.id] = TriggerState::RISING;
				}
				else if (triggerStates[counter.id] == TriggerState::RISING) {
					triggerStates[counter.id] = TriggerState::ON;
				}
			}
			else {
				triggerStates[counter.id] = TriggerState::OFF;
			}
		}

		for (int i = 0; i < areas.size(); i++) {
			Area area = areas.at(i);
			if (!area.enabled) continue;
			switch (area.type) {
			case Area::CIRCULAR_AREA:
			{
				Point2f shock_point = points.robot.pt;
				shock_point.x += area.distance * sin((points.robot.angle + area.angle)*CV_PI / 180);
				shock_point.y -= area.distance * cos((points.robot.angle + area.angle)*CV_PI / 180);
				double distance = cv::norm(points.rat.pt - shock_point);
				if (distance < area.radius) {
					activeTriggers.append(area.id);
					found = true;
					if (triggerStates[area.id] == TriggerState::OFF) {
						triggerStates[area.id] = TriggerState::RISING;
					}
					else if (triggerStates[area.id] == TriggerState::RISING) {
						triggerStates[area.id] = TriggerState::ON;
					}
				}
				else {
					triggerStates[area.id] = TriggerState::OFF;
				}
			}
			break;
			case Area::PIE_AREA:
				qreal angleRad = qAtan2(arena.y - points.rat.pt.y, arena.x - points.rat.pt.x);
				int angle = fmod(qRadiansToDegrees(angleRad) + 270, 360);
				if (angle > area.angle - area.range / 2 && angle < area.angle + area.range / 2) {
					activeTriggers.append(area.id);
					if (triggerStates[area.id] == TriggerState::OFF) {
						triggerStates[area.id] = TriggerState::RISING;
					}
					else if (triggerStates[area.id] == TriggerState::RISING) {
						triggerStates[area.id] = TriggerState::ON;
					}
				}
				else {
					triggerStates[area.id] = TriggerState::OFF;
				}
				break;
			}
		}
	}

	// Process triggers
	bool shocking = false;
	for (int i = 0; i < activeTriggers.size(); i++) {
		QString trigger = activeTriggers.at(i);

		for (int j = 0; j < actions.size(); j++) {
			Action action = actions.at(j);
			if (trigger == action.trigger) {
				switch (action.type) {
				case Action::SHOCK:
					shocking = true;
					break;
				case Action::ENABLE:
				case Action::DISABLE:
				{
					bool found = false;
					for (int k = 0; k < areas.size(); k++) {
						Area& area = areas[k];
						if (action.target == area.id) {
							if (action.type == Action::ENABLE) {
								logger->log(QString("Enabling area \"%1\"").arg(area.id));
								area.enabled = true;
							}
							else {
								logger->log(QString("Disabling area \"%1\"").arg(area.id));
								area.enabled = false;
							}
							found = true;
							break;
						}
					}
					if (found) break;
					for (int k = 0; k < counters.size(); k++) {
						Counter& counter = counters[k];
						if (action.target == counter.id) {
							if (action.type == Action::ENABLE) {
								logger->log(QString("Starting counter \"%1\"").arg(counter.id));
								counter.active = true;
							}
							else {
								logger->log(QString("Stopping counter \"%1\"").arg(counter.id));
								counter.active = false;
							}
							break;
						}
					}
					break;
				}
				case Action::COUNTER_INC:
				case Action::COUNTER_DEC:
				case Action::COUNTER_SET:
					if (triggerStates[trigger] != TriggerState::RISING) continue;
					for (int i = 0; i < counters.size(); i++) {
						Counter& counter = counters[i];
						if (action.target == counter.id) {
							if (action.type == Action::COUNTER_INC) {
								counter.value += action.arg;
								logger->log(QString("Increasing counter \"%1\" by %2").arg(counter.id).arg(action.arg));
							}
							else if (action.type == Action::COUNTER_DEC) {
								counter.value -= action.arg;
								logger->log(QString("Decreasing counter \"%1\" by %2").arg(counter.id).arg(action.arg));
							}
							else {
								counter.value = action.arg;
								logger->log(QString("Setting counter \"%1\" to %2").arg(counter.id).arg(action.arg));
							}
							break;
						}
					}
					break;
				case Action::FEEDER:
					if (triggerStates[trigger] != TriggerState::RISING) continue;
					if (isLive) {
						Arenomat* mat = dynamic_cast<Arenomat*>(hardware.get());
						mat->feed();
					}
					stats.feederCount++;
					break;
				}
			}
		}
	}


	switch (shockState) {
	case OUTSIDE:
		if (shocking) {
			shockState = DELAYING;
			stats.entryCount++;
			lastChange = elapsedTimer.elapsed();
		}
		break;
	case DELAYING:
		if (!badFrame) {
			if (shocking) {
				if (elapsedTimer.elapsed() > lastChange + shock.delay) {
					shockState = SHOCKING;
					stats.shockCount++;
					if (stats.initialShock == 0) {
						stats.initialShock = elapsedTimer.elapsed();
					}
					lastChange = elapsedTimer.elapsed();
					outputShock(shockLevel);
				}
			}
			else {
				shockState = OUTSIDE;
			}
		}
		break;
	case SHOCKING:
		if (elapsedTimer.elapsed() > lastChange + shock.length) {
			outputShock(0);
			if (!badFrame) {
				if (shocking) {
					shockState = PAUSE;
				}
				else {
					shockState = REFRACTORY;
				}
				lastChange = elapsedTimer.elapsed();
			}
		}
		break;
	case PAUSE:
		if (!badFrame) {
			if (shocking) {
				if (elapsedTimer.elapsed() > lastChange + shock.in_delay) {
					shockState = SHOCKING;
					stats.shockCount++;
					lastChange = elapsedTimer.elapsed();
					outputShock(shockLevel);
				}
			}
			else {
				shockState = REFRACTORY;
				lastChange = elapsedTimer.elapsed();
			}
		}
		break;
	case REFRACTORY:
		if (elapsedTimer.elapsed() > lastChange + shock.refractory) {
			shockState = OUTSIDE;
		}
	}

	ExperimentState state;
	state.ts = timestamp;
	state.frame = frame;
	state.rat = points.rat;
	state.robot = points.robot;
	state.stats = stats;
	state.areas = areas;
	state.counters = counters;
	state.state = shockState;
	state.shock = shockState == SHOCKING ? currentShockLevel : 0;
	emit(update(state));

	if (synchOut) hardware->setSync(synchInv);
}

void Experiment::setShockLevel(int level) {
	if (level < 2) {
		shockLevel = 0;
	}
	else if (level > 7) {
		shockLevel = 7;
	}
	else {
		shockLevel = level;
	}
	logger->log(QString("Shock level set to %1 mA").arg(shockLevel));
}

void Experiment::outputShock(int level) {
	if (!shockIsElectric && isLive) {
		Arenomat* mat = dynamic_cast<Arenomat*>(hardware.get());
		mat->setLight(level > 0);
	}
	else {
		hardware->setShock(level);
	}
	logger->log(QString("Outputting shock at %1 mA").arg(level));
}

QString Experiment::getLog() {
	return experimentLogger->get(finishedTime);
}
