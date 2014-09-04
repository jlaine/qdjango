include(../../qdjango.pri)
include(../../tests/tests.pri)

QT += sql
LIBS += -L$${DEPTH}/src/db $${QDJANGO_DB_LIBS}
INCLUDEPATH += $${PWD}

HEADERS += \
    $$PWD/util.h
SOURCES += \
    $$PWD/util.cpp
