include(../../qdjango.pri)

TEMPLATE = lib
CONFIG += $$QDJANGO_LIBRARY_TYPE
DEFINES += QDJANGO_BUILD
QT += script sql

TARGET = qdjango-script
VERSION = $$QDJANGO_VERSION
win32 {
    DESTDIR = $$OUT_PWD
}

INCLUDEPATH += ../db
HEADERS += QDjangoScript.h
SOURCES += QDjangoScript.cpp
