#include "experimentlogger.h"
#include <QDateTime>

ExperimentLogger::ExperimentLogger(Shock shock, Arena arena, QList<Area> areas, QList<Counter> counters, QList<Action> actions, QObject *parent) : QObject(parent)
{
	this->shock = shock;
	this->arena = arena;
	this->areas = areas;
	this->counters = counters;
	this->actions = actions;
}

void ExperimentLogger::setStart(qint64 timestamp) {
	this->startTime = timestamp;
}

void ExperimentLogger::add(ExperimentState state) {
	Frame frame;
	frame.rat = state.rat;
	frame.robot = state.robot;
	frame.state = state.state;
	frame.shock = state.shock;
	frame.counters = state.counters;
	frame.areas = state.areas;
	frame.timestamp = state.ts;
	frame.position = state.position;
	frames.append(frame);

	if (state.robot.valid) {
		hasRobot = true;
	}

	if (state.rat.angle != -1) {
		hasAngle = true;
	}
}

QString ExperimentLogger::get(qint64 elapsedTime) {
	QDateTime start;
	start.setMSecsSinceEpoch(startTime);

	QString log;
	log += QString("%%BEGIN_HEADER\r\n");
	log += QString("// Date ( %1 )\r\n").arg(start.toString("d.M.yyyy"));
	log += QString("// Time ( %1 )\r\n").arg(start.toString("h:mm"));
	log += QString("  %MilliTime ( %1 )\r\n").arg(startTime);
	log += QString("  %ElapsedTime_ms ( %1 )\r\n").arg(elapsedTime);
	// log += QString("  %Paradigm ( %1 )\r\n").arg(paradigm);
	log += QString("\r\n");
	log += QString("  %ShockDelay ( %1 )\r\n").arg(shock.delay);
	log += QString("  %ShockLength ( %1 )\r\n").arg(shock.length);
	log += QString("  %ShockPause ( %1 )\r\n").arg(shock.in_delay);
	log += QString("  %ShockRefractory ( %1 )\r\n").arg(shock.refractory);
	log += QString("\r\n");
	log += QString("  %ArenaDiameter_m ( %1 )\r\n").arg(arena.size);
	log += QString("  %ArenaCenterXY ( %1 %2 )\r\n").arg(arena.x).arg(arena.y);
	log += QString("  %TrackerResolution_PixPerCM.0 ( %1 )\r\n").arg((2 * arena.radius) / (arena.size * 100));
	log += QString("\r\n");

	int index = 0;

	for (Area area : areas) {
		if (area.type == area.CIRCULAR_AREA) {
			log += QString("  %RobotZone ( %1 %2 %3 %4 %5 )\r\n").arg(index++).arg(area.enabled).arg(area.radius).arg(area.angle).arg(area.distance);
		}
		else {
			log += QString("  %ArenaZone ( %1 %2 %3 %4 )\r\n").arg(index++).arg(area.enabled).arg(area.angle).arg(area.range);
		}
	}

	for (Counter counter : counters) {
		log += QString("  %Counter ( %1 %2 %3 %4 %5 )\r\n").arg(index++).arg(counter.active ? 1 : 0)
			.arg(counter.singleShot ? 1 : 0).arg(counter.period).arg(counter.limit);
	}

	for (Action action : actions) {
		QString actionLine("  %Action ( %1 %2 %3 )\r\n");
		actionLine = actionLine.arg(action.trigger);

		switch (action.type)
		{
		case action.SHOCK:
			actionLine = actionLine.arg("SHOCK");
			break;
		case action.ENABLE:
			actionLine = actionLine.arg("ENABLE");
			break;
		case action.DISABLE:
			actionLine = actionLine.arg("DISABLE");
			break;
		case action.COUNTER_INC:
			actionLine = actionLine.arg("COUNTER_INC");
			break;
		case action.COUNTER_DEC:
			actionLine = actionLine.arg("COUNTER_DEC");
			break;
		case action.COUNTER_SET:
			actionLine = actionLine.arg("COUNTER_SET");
			break;
		case action.SOUND:
			actionLine = actionLine.arg("SOUND");
			break;
		case action.LIGHT_ON:
			actionLine = actionLine.arg("LIGHT_ON");
			break;
		case action.LIGHT_OFF:
			actionLine = actionLine.arg("LIGHT_OFF");
			break;
		case action.FEEDER:
			actionLine = actionLine.arg("FEEDER");
			break;
		}
		actionLine = actionLine.arg(action.arg);

		log += actionLine;
	}


	QString rowLine("  %Row ( FrameCount 1msTimeStamp RatRoomX RatRoomY ");
	if (hasAngle) {
		rowLine += "RatAngle ";
	}

	if (hasRobot) {
		rowLine += "RobotRoomX RobotRoomY ";
		if (hasAngle) {
			rowLine += "RobotAngle ";
		}
	}

	rowLine += "State CurrentLevel Position";

	index = 0;

	for (Area area : areas) {
		rowLine += QString("%1_Enabled ").arg(index++);
	}

	for (Counter counter : counters) {
		rowLine += QString("%1_Active %1_Value ").arg(index++);
	}

	rowLine += ")\r\n";

	log += rowLine;
	log += QString("  %TrackerVersion ( %1 )\r\n").arg(KACHNA_VERSION);
	log += QString("%%END_HEADER\r\n");

	int i = 1;
	auto it = frames.begin();
	while (it != frames.end()) {
		Frame frame = (*it);
		log += QString::number(i);
		log += "    ";
		log += QString::number(frame.timestamp);
		log += "    ";
		log += QString::number(frame.rat.pt.x);
		log += "    ";
		log += QString::number(frame.rat.pt.y);
		log += "    ";
		if (hasAngle) {
			log += QString::number(frame.rat.angle);
			log += "    ";
		}
		if (hasRobot) {
			log += QString::number(frame.robot.pt.x);
			log += "    ";
			log += QString::number(frame.robot.pt.y);
			log += "    ";
			if (hasAngle) {
				log += QString::number(frame.robot.angle);
				log += "    ";
			}
		}
		log += QString::number(frame.state);
		log += "    ";
		log += QString::number(frame.shock);
		log += "    ";
		log += QString::number(frame.position);
		log += "    ";

		for (Area area : frame.areas) {
			log += QString("%1    ").arg(area.enabled ? 1 : 0);
		}

		for (Counter counter : frame.counters) {
			log += QString("%1    %2    ").arg(counter.active ? 1 : 0).arg(counter.value);
		}

		log += "\r\n";

		i++;
		it++;
	}

	return log;
}
