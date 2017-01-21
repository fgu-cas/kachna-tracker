INCLUDEPATH += $${PWD}
DEPENDPATH += $${PWD}

SOURCES +=  $$PWD/detector.cpp \
    $$PWD/detector_color.cpp \
    $$PWD/detector_threshold.cpp \
    $$PWD/experiment.cpp \
    $$PWD/arenomat.cpp \
    $$PWD/experimentlogger.cpp \
    $$PWD/abstracthardware.cpp \
    $$PWD/dio24.cpp \
    $$PWD/params.cpp \
    $$PWD/dummyhardware.cpp

HEADERS += $$PWD/detector.h \
    $$PWD/detector_color.h \
    $$PWD/detector_threshold.h \
    $$PWD/experiment.h \
    $$PWD/action.h \
    $$PWD/arenomat.h \
    $$PWD/experimentlogger.h \
    $$PWD/params.h \
    $$PWD/abstracthardware.h \
    $$PWD/dio24.h \
    $$PWD/dummyhardware.h
