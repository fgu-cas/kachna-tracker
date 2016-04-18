INCLUDEPATH += $${PWD}
DEPENDPATH += $${PWD}

SOURCES += $$PWD/configwindow.cpp \
    $$PWD/mainwindow.cpp \
    $$PWD/masklabel.cpp \
    $$PWD/pointdialog.cpp \
    $$PWD/pointselector.cpp \
    $$PWD/shockspinbox.cpp \
    $$PWD/configwindow_general.cpp \
    $$PWD/configwindow_tracking.cpp \
    $$PWD/configwindow_capture.cpp \
    $$PWD/configwindow_actions.cpp \
    $$PWD/actions_actionwidget.cpp \
    $$PWD/actions_areawidget.cpp \
    $$PWD/actions_counterwidget.cpp \
    $$PWD/actions_area_robot_dialog.cpp \
    $$PWD/actions_area_sector_dialog.cpp \
    $$PWD/shockrobotlabel.cpp \
    $$PWD/shocksectorlabel.cpp \
    $$PWD/actions_modifycounter_dialog.cpp

HEADERS  += $$PWD/configwindow.h \
    $$PWD/mainwindow.h \
    $$PWD/masklabel.h \
    $$PWD/pointdialog.h \
    $$PWD/pointselector.h \
    $$PWD/shockspinbox.h \
    $$PWD/actions_actionwidget.h \
    $$PWD/actions_areawidget.h \
    $$PWD/actions_counterwidget.h \
    $$PWD/actions_area_robot_dialog.h \
    $$PWD/actions_area_sector_dialog.h \
    $$PWD/shockrobotlabel.h \
    $$PWD/shocksectorlabel.h \
    $$PWD/actions_modifycounter_dialog.h

FORMS    += $$PWD/configwindow.ui \
    $$PWD/mainwindow.ui \
    $$PWD/actions_area_robot_dialog.ui \
    $$PWD/actions_area_sector_dialog.ui \
    $$PWD/actions_modifycounter_dialog.ui
