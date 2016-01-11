#-------------------------------------------------
#
# Project created by QtCreator 2014-01-28T15:46:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kachna-tracker
TEMPLATE = app

QMAKE_CXXFLAGS += "-std=c++11"
CXXFLAGS = "-std=c++0x"

SOURCES += main.cpp\
        kachnatracker.cpp \
    configwindow.cpp \
    experiment.cpp \
    detector.cpp \
    shockspinbox.cpp \
    masklabel.cpp \
    detector_threshold.cpp \
    detector_color.cpp \
    colordialog.cpp \
    colorselector.cpp

HEADERS  += kachnatracker.h \
    configwindow.h \
    experiment.h \
    shockspinbox.h \
    detector.h \
    masklabel.h \
    detector_threshold.h \
    detector_color.h \
    colordialog.h \
    colorselector.h

FORMS    += configwindow.ui \
    kachnatracker.ui

INCLUDEPATH += C:\\dev\\opencv\\build\\include

LIBS += -LC:\\dev\\opencv\\build\\bin \
    libopencv_core2411d \
    libopencv_highgui2411d \
    libopencv_imgproc2411d \
    libopencv_features2d2411d \
    libopencv_calib3d2411d

LIBS += -L .. \
        -lcbw32

LIBS += -lole32 -loleaut32

QMAKE_LFLAGS = -static-libgcc -static-libstdc++
