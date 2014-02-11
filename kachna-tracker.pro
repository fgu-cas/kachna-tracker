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
    detectordialog.cpp \
    util.cpp

HEADERS  += kachnatracker.h \
    detectordialog.h

FORMS    += kachnatracker.ui \
    detectordialog.ui

LIBS += `pkg-config opencv --libs`
