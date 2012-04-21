include(../../qdjango.pri)

TEMPLATE = lib
CONFIG += $$QDJANGO_LIBRARY_TYPE
DEFINES += QDJANGO_BUILD
QT += sql

TARGET = qdjango-db
VERSION = $$QDJANGO_VERSION
win32 {
    DESTDIR = $$OUT_PWD
}

HEADERS += \
    QDjango.h \
    QDjango_p.h \
    QDjangoModel.h \
    QDjangoQuerySet.h \
    QDjangoQuerySet_p.h \
    QDjangoWhere.h
SOURCES += \
    QDjango.cpp \
    QDjangoModel.cpp \
    QDjangoQuerySet.cpp \
    QDjangoWhere.cpp
