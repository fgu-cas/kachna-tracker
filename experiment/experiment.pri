INCLUDEPATH += $${PWD}
DEPENDPATH += $${PWD}

SOURCES +=  $$PWD/detector.cpp \
    $$PWD/detector_color.cpp \
    $$PWD/detector_threshold.cpp \
    $$PWD/experiment.cpp \
    $$PWD/shockmanager.cpp \
    $$PWD/arenomat.cpp

HEADERS += $$PWD/detector.h \
    $$PWD/detector_color.h \
    $$PWD/detector_threshold.h \
    $$PWD/experiment.h \
    $$PWD/shockmanager.h \
    $$PWD/action.h \
    $$PWD/arenomat.h
