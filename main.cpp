#include "kachnatracker.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    kachnatracker window;
    window.show();

    return app.exec();
}


