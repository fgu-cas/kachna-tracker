#include "kachnatracker.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    kachnatracker w;
    w.show();

    return a.exec();
}


