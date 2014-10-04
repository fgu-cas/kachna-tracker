#-------------------------------------------------
#
# Project created by QtCreator 2014-01-28T15:46:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kachna-tracker
TEMPLATE = app


SOURCES += main.cpp\
        kachnatracker.cpp \
    configwindow.cpp \
    experiment.cpp \
    blobdetector.cpp \
    shockspinbox.cpp

HEADERS  += kachnatracker.h \
    configwindow.h \
    experiment.h \
    blobdetector.h \
    shockspinbox.h

FORMS    += configwindow.ui \
    kachnatracker.ui

INCLUDEPATH += C:\\dev\\opencv\\build\\include

LIBS += -LC:\\dev\\opencv\\build\\bin \
    libopencv_core249d \
    libopencv_highgui249d \
    libopencv_imgproc249d \
    libopencv_features2d249d \
    libopencv_calib3d249d

LIBS += -L .. \
        -lcbw32

QMAKE_LFLAGS = -static-libgcc -static-libstdc++
