QT -= gui
QT += sql

TARGET = qdjango-db
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

# Installation
include(../src.pri)
headers.path = $$PREFIX/include/qdjango/db
QMAKE_PKGCONFIG_INCDIR = $$headers.path
