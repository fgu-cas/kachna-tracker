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
    blobdetector.cpp

HEADERS  += kachnatracker.h \
    configwindow.h \
    experiment.h \
    blobdetector.h

FORMS    += configwindow.ui \
    kachnatracker.ui
