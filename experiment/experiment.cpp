#include "experiment.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QDateTime>
#include "action.h"
#include "arenomat.h"
#include "dummyhardware.h"


Experiment::Experiment(QObject *parent, QMap<QString, QVariant>  *settings) :
    QObject(parent){
    int deviceIndex = settings->value("video/device").toInt();
    if (deviceIndex == -1){
        capture.open(settings->value("video/filename").toString().toStdString());
        isLive = false;
    } else {
        capture.open(deviceIndex);
        isLive = true;
    }

    if (settings->value("tracking/type").toInt() == 0){
        detector.reset(new DetectorThreshold(*settings, capture.get(CV_CAP_PROP_FRAME_HEIGHT), capture.get(CV_CAP_PROP_FRAME_WIDTH)));
    } else {
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

    shockLevel = settings->value("shock/initialShock").toInt();
    shock.delay = settings->value("shock/EntranceLatency").toInt();
    shock.in_delay = settings->value("shock/InterShockLatency").toInt();
    shock.length = settings->value("shock/ShockDuration").toInt();
    shock.refractory = settings->value("shock/OutsideRefractory").toInt();

    arenaPWM = settings->value("arena/PWM").toInt();

    QString direction = settings->value("arena/direction").toString();
    if (direction == "CW"){
        arenaDirection = 1;
    } else if (direction == "CCw"){
        arenaDirection = 2;
    } else {
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

    logger.reset(new ExperimentLogger(shock, arena));
    if (isLive){
        hardware.reset(new Arenomat(settings->value("hardware/serialPort").toString()));
    } else {
        hardware.reset(new DummyHardware());
    }
}

Experiment::~Experiment(){
    hardware->setShock(0);
}

void Experiment::start(){
    timer.start();
    elapsedTimer.start();
    logger->setStart(QDateTime::currentMSecsSinceEpoch());

    if (arenaDirection > 0){
        Arenomat* mat = dynamic_cast<Arenomat*>(hardware.get());
        mat->setTurntableDirection(arenaDirection);
        mat->setTurntablePWM(arenaPWM);
    }
}

void Experiment::stop(){
    hardware->setShock(0);
    this->timer.stop();
    capture.release();
    finishedTime = elapsedTimer.elapsed();
}

bool Experiment::isRunning(){
    return this->timer.isActive();
}

void Experiment::processFrame(){
    // Output sync signal
    if (synchOut) hardware->setSync(!synchInv);

    // Process counters

    for (int i = 0; i < counters.size(); i++){
        Counter& counter = counters[i];
        if (counter.enabled && (elapsedTimer.elapsed() - counter.lastUpdated) > counter.frequency*1000){
            counter.lastUpdated = elapsedTimer.elapsed();
            counter.value++;
        }
    }


    //Process the frame
    Mat frame;
    bool read = capture.read(frame);
    if ( !read && !isLive ){ //We've reached the end of the file
        kachnatracker* mainwindow = dynamic_cast<kachnatracker*>(parent());
        mainwindow->experimentTimeout();
        return;
    }
    qint64 timestamp = elapsedTimer.elapsed();
    lastFrame = frame;

    // Detect points
    Detector::pointPair points = detector->find(&frame);
    if (!ratRobot) points.robot = Detector::Point();
    bool badFrame = points.rat.size == -1 || (ratRobot && points.robot.size == -1);
    if (skip_reaction == 1){
        if (lastPoints.rat.size == -1 ||
                cv::norm(lastPoints.rat.pt - points.rat.pt) < skip_distance ||
                ratTimer.elapsed() > skip_timeout){
            lastPoints.rat = points.rat;
            ratTimer.start();
        } else {
            points.rat = Detector::Point();
        }

        if (lastPoints.robot.size == -1 ||
                cv::norm(lastPoints.robot.pt - points.robot.pt) < skip_distance ||
                robotTimer.elapsed() > skip_timeout){
            lastPoints.robot = points.robot;
            robotTimer.start();
        } else {
            points.robot = Detector::Point();
        }
    }
    lastKeypoints = points;

    // Find active triggers
    QStringList activeTriggers;
    bool found;
    if (badFrame){
        stats.badFrames++;

        for (int i = 0; i < counters.size(); i++){
            Counter counter = counters.at(i);
            if (counter.value >= counter.limit){
                activeTriggers.append(counter.id);
            }
        }
    } else {
        stats.goodFrames++;

        for (int i = 0; i < counters.size(); i++){
            Counter counter = counters.at(i);
            if (counter.value >= counter.limit){
                activeTriggers.append(counter.id);
            }
        }

        for (int i = 0; i < areas.size(); i++){
            Area area = areas.at(i);
            switch (area.type){
            case Trigger::CIRCULAR_AREA:
                if (!area.enabled) continue;
            {
                Point2f shock_point = points.robot.pt;
                shock_point.x += area.distance * sin((points.robot.angle + area.angle)*CV_PI/180);
                shock_point.y -= area.distance * cos((points.robot.angle + area.angle)*CV_PI/180);
                double distance = cv::norm(points.rat.pt - shock_point);
                if (distance < area.radius){
                    activeTriggers.append(area.id);
                    found = true;
                }
            }
                break;
            case Trigger::PIE_AREA:
                qreal angleRad = qAtan2(arena.y - points.rat.pt.y, arena.x - points.rat.pt.x);
                int angle = fmod(qRadiansToDegrees(angleRad)+ 270, 360);
                if (angle > area.angle-area.range/2 && angle < area.angle+area.range/2){
                    activeTriggers.append(area.id);
                }
                break;
            }
        }
    }

    // Process triggers
    bool shocking = false;
    for (int i = 0; i < activeTriggers.size(); i++){
        QString trigger = activeTriggers.at(i);

        for (int j = 0; j < actions.size(); j++){
            Action action = actions.at(j);
            if (trigger == action.trigger){
                switch (action.type){
                case Action::SHOCK:
                    shocking = true;
                    break;
                case Action::ENABLE:
                case Action::DISABLE:
                {
                    bool found = false;
                    for (int k = 0; k < areas.size(); k++){
                        Area& area = areas[k];
                        if (action.target == area.id){
                            area.enabled = action.type == Action::ENABLE ? true : false;
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                    for (int k = 0; k < counters.size(); k++){
                        Counter& counter = counters[k];
                        if (action.target == counter.id){
                            counter.enabled = action.type == Action::ENABLE ? true : false;
                            break;
                        }
                    }
                    break;
                }
                case Action::COUNTER_INC:
                case Action::COUNTER_DEC:
                case Action::COUNTER_SET:
                    action.processed = true;
                    for (int i = 0; i < counters.size(); i++){
                        Counter& counter = counters[i];
                        if (action.target == counter.id){
                            if (action.type == Action::COUNTER_INC){
                                counter.value += action.arg;
                            } else if(action.type == Action::COUNTER_DEC){
                                counter.value -= action.arg;
                            } else {
                                counter.value = action.arg;
                            }
                            break;
                        }
                    }
                    break;

                }
            }
        }
    }


    switch (shockState){
    case OUTSIDE:
        if (shocking){
            shockState = DELAYING;
            stats.entryCount++;
            lastChange = elapsedTimer.elapsed();
        }
        break;
    case DELAYING:
        if (!badFrame){
            if (shocking){
                if (elapsedTimer.elapsed() > lastChange+shock.delay){
                    shockState = SHOCKING;
                    stats.shockCount++;
                    if (stats.initialShock == 0){
                        stats.initialShock = elapsedTimer.elapsed();
                    }
                    lastChange = elapsedTimer.elapsed();
                    hardware->setShock(shockLevel);
                }
            } else {
                shockState=OUTSIDE;
            }
        }
        break;
    case SHOCKING:
        if (elapsedTimer.elapsed() > lastChange+shock.length){
            hardware->setShock(0);
            if (!badFrame){
                if (shocking){
                    shockState = PAUSE;
                } else {
                    shockState = REFRACTORY;
                }
                lastChange = elapsedTimer.elapsed();
            }
        }
        break;
    case PAUSE:
        if (!badFrame){
            if (shocking){
                if (elapsedTimer.elapsed() > lastChange+shock.in_delay){
                    shockState = SHOCKING;
                    stats.shockCount++;
                    lastChange = elapsedTimer.elapsed();
                    hardware->setShock(shockLevel);
                }
            } else {
                shockState = REFRACTORY;
                lastChange = elapsedTimer.elapsed();
            }
        }
        break;
    case REFRACTORY:
        if (elapsedTimer.elapsed() > lastChange+shock.refractory){
            shockState = OUTSIDE;
        }
    }

    logger->add(points.rat, found ? 1 : 0, shockState, shockState == SHOCKING ? currentShockLevel : 0, timestamp);
    logger->add(points.robot, found ? 1 : 0, shockState, shockState == SHOCKING ? currentShockLevel : 0, timestamp);

    if (synchOut) hardware->setSync(synchInv);
}

void Experiment::setShockLevel(int level){
    if (level < 2){
        shockLevel = 0;
    } else if (level > 7){
        shockLevel = 7;
    } else {
        shockLevel = level;
    }
}

QList<Area> Experiment::getAreas(){
    return areas;
}

Experiment::Update Experiment::getUpdate(){
    Update update;

    update.keypoints = lastKeypoints;
    update.stats = stats;
    update.frame = lastFrame;

    return update;
}

QString Experiment::getLog(bool rat){
    Detector::CLASS_ID id;
    if (rat){
        id = Detector::RAT;
    } else {
        id = Detector::ROBOT;
    }

    return logger->get(id, finishedTime);
}
