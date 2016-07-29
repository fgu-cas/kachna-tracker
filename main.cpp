#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qRegisterMetaType<Action>("Action");
    qRegisterMetaTypeStreamOperators<Action>("Action");
    QMetaType::registerEqualsComparator<QList<Action>>();
    qRegisterMetaTypeStreamOperators<QList<Action>>("QList<Action>");

    qRegisterMetaType<Area>("Area");
    qRegisterMetaTypeStreamOperators<Area>("Area");
    QMetaType::registerEqualsComparator<QList<Area>>();
    qRegisterMetaTypeStreamOperators<QList<Area>>("QList<Area>");

    qRegisterMetaType<Counter>("Counter");
    qRegisterMetaTypeStreamOperators<Counter>("Counter");
    QMetaType::registerEqualsComparator<QList<Counter>>();
    qRegisterMetaTypeStreamOperators<QList<Counter>>("QList<Counter>");

    kachnatracker window;
    window.show();

    return app.exec();
}


