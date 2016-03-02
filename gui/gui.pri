INCLUDEPATH += $${PWD}
DEPENDPATH += $${PWD}

SOURCES += $$PWD/configwindow.cpp \
    $$PWD/mainwindow.cpp \
    $$PWD/masklabel.cpp \
    $$PWD/pointdialog.cpp \
    $$PWD/pointselector.cpp \
    $$PWD/shocklocationlabel.cpp \
    $$PWD/shockspinbox.cpp \
    $$PWD/configwindow_general.cpp \
    $$PWD/configwindow_tracking.cpp \
    $$PWD/configwindow_capture.cpp \
    $$PWD/configwindow_actions.cpp \
    $$PWD/actions_actionwidget.cpp \
    $$PWD/actions_areawidget.cpp \
    $$PWD/actions_counterwidget.cpp

HEADERS  += $$PWD/configwindow.h \
    $$PWD/mainwindow.h \
    $$PWD/masklabel.h \
    $$PWD/pointdialog.h \
    $$PWD/pointselector.h \
    $$PWD/shocklocationlabel.h \
    $$PWD/shockspinbox.h \
    $$PWD/actions_actionwidget.h \
    $$PWD/actions_areawidget.h \
    $$PWD/actions_counterwidget.h

FORMS    += $$PWD/configwindow.ui \
    $$PWD/mainwindow.ui
