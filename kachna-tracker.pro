QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kachna-tracker
TEMPLATE = app

QMAKE_CXXFLAGS += "-std=c++11"
CXXFLAGS = "-std=c++0x"


SOURCES += main.cpp

include($$PWD/gui/gui.pri)

include($$PWD/experiment/experiment.pri)

INCLUDEPATH += "$$PWD\\..\\opencv\\build\\include"

LIBS += -L"$$PWD\\..\\opencv\\build\\bin" \
    libopencv_core2413d \
    libopencv_highgui2413d \
    libopencv_imgproc2413d \
    libopencv_features2d2413d \
    libopencv_calib3d2413d

LIBS += -lole32 -loleaut32

QMAKE_LFLAGS = -static-libgcc -static-libstdc++

RESOURCES +=
