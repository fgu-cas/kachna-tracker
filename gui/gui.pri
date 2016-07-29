INCLUDEPATH += $${PWD}
DEPENDPATH += $${PWD}

SOURCES += $$PWD/configwindow.cpp \
    $$PWD/mainwindow.cpp \
    $$PWD/masklabel.cpp \
    $$PWD/pointdialog.cpp \
    $$PWD/pointselector.cpp \
    $$PWD/configwindow_general.cpp \
    $$PWD/configwindow_tracking.cpp \
    $$PWD/configwindow_capture.cpp \
    $$PWD/configwindow_actions.cpp \
    $$PWD/actions_area_robot_dialog.cpp \
    $$PWD/actions_area_sector_dialog.cpp \
    $$PWD/shockrobotlabel.cpp \
    $$PWD/shocksectorlabel.cpp \
    $$PWD/actions_modifycounter_dialog.cpp \
    $$PWD/actions_enabledisabledialog.cpp \
    $$PWD/actionssounddialog.cpp \
    $$PWD/debugwindow.cpp

HEADERS  += $$PWD/configwindow.h \
    $$PWD/mainwindow.h \
    $$PWD/masklabel.h \
    $$PWD/pointdialog.h \
    $$PWD/pointselector.h \
    $$PWD/actions_area_robot_dialog.h \
    $$PWD/actions_area_sector_dialog.h \
    $$PWD/shockrobotlabel.h \
    $$PWD/shocksectorlabel.h \
    $$PWD/actions_modifycounter_dialog.h \
    $$PWD/actions_enabledisabledialog.h \
    $$PWD/actionssounddialog.h \
    $$PWD/debugwindow.h

FORMS    += $$PWD/configwindow.ui \
    $$PWD/mainwindow.ui \
    $$PWD/actions_area_robot_dialog.ui \
    $$PWD/actions_area_sector_dialog.ui \
    $$PWD/actions_modifycounter_dialog.ui \
    $$PWD/actions_enabledisabledialog.ui \
    $$PWD/actionssounddialog.ui \
    $$PWD/debugwindow.ui

RESOURCES += \
    $$PWD/resources.qrc
