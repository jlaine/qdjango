include(../../qdjango.pri)

QT -= gui
QT += script sql

DEFINES += QDJANGO_SCRIPT_BUILD

TARGET = qdjango-script
win32 {
    DESTDIR = $$OUT_PWD
}

INCLUDEPATH += ../db
LIBS += -L../db $$QDJANGO_DB_LIBS
HEADERS += QDjangoScript.h QDjangoScript_p.h
SOURCES += QDjangoScript.cpp

# Installation
include(../src.pri)
headers.path = $$PREFIX/include/qdjango/script
QMAKE_PKGCONFIG_INCDIR = $$headers.path
