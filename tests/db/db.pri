include(../tests.pri)

QT += sql

HEADERS += $$PWD/util.h
SOURCES += $$PWD/util.cpp

INCLUDEPATH += $$PWD
LIBS += -L../../../src/db $$QDJANGO_DB_LIBS
