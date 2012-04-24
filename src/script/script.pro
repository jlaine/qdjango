QT += script sql

TARGET = qdjango-script
win32 {
    DESTDIR = $$OUT_PWD
}

INCLUDEPATH += ../db
LIBS += -lqdjango-db
HEADERS += QDjangoScript.h
SOURCES += QDjangoScript.cpp

# Installation
include(../src.pri)
headers.path = $$PREFIX/include/qdjango/script
