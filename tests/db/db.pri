include(../../qdjango.pri)

QT -= gui
QT += sql testlib

HEADERS += $$PWD/util.h
SOURCES += $$PWD/util.cpp
INCLUDEPATH += $$PWD $$QDJANGO_INCLUDEPATH

LIBS += -L../../../src/db $$QDJANGO_DB_LIBS
