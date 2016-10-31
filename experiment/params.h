#ifndef PARAMS_H
#define PARAMS_H

#include <QString>
#include <QVariant>
#include <opencv2/core/core.hpp>

struct Shock {
    int length;
    int delay;
    int in_delay;
    int refractory;
};

struct Arena {
    int x;
    int y;
    double size;
    int radius;
};

struct Area {
	QString id = "";
	bool enabled = true;
    int radius = 0;
    int distance = 0;
    int angle = 0;
    int range = 0;
	enum TYPE {
		CIRCULAR_AREA,
		PIE_AREA
	} type = CIRCULAR_AREA;
    bool operator==(const Area &other) const;
}; Q_DECLARE_METATYPE(Area)

QDataStream &operator<<(QDataStream &in, const Area &myObj);
QDataStream &operator>>(QDataStream &in, Area &myObj);

struct Counter {
	QString id = "";
	bool active = false;
	bool singleShot = false;
    int value = 0;
    double period = 0;
    int limit = 0;
    qint64 lastUpdated = 0;
    bool operator==(const Counter &other) const;
}; Q_DECLARE_METATYPE(Counter)

QDataStream &operator<<(QDataStream &in, const Counter &myObj);
QDataStream &operator>>(QDataStream &in, Counter &myObj);

struct Action {
    QString trigger;
    bool processed = false;
    enum TYPE {
        SHOCK,
        ENABLE,
        DISABLE,
        COUNTER_INC,
        COUNTER_DEC,
        COUNTER_SET,
        SOUND,
        LIGHT,
        FEEDER
    } type;
    QString target;
    int arg = 0;
    bool operator==(const Action &other) const;
}; Q_DECLARE_METATYPE(Action)


QDataStream &operator<<(QDataStream &in, const Action &myObj);
QDataStream &operator>>(QDataStream &in, Action &myObj);



#endif // PARAMS_H
