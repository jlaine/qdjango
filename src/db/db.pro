include(../../qdjango.pri)

QT -= gui
QT += sql

DEFINES += QDJANGO_DB_BUILD

TARGET = qdjango-db
win32 {
    DESTDIR = $$OUT_PWD
}

HEADERS += \
    QDjango.h \
    QDjango_p.h \
    QDjangoMetaModel.h \
    QDjangoModel.h \
    QDjangoQuerySet.h \
    QDjangoQuerySet_p.h \
    QDjangoWhere.h \
    QDjangoWhere_p.h
SOURCES += \
    QDjango.cpp \
    QDjangoMetaModel.cpp \
    QDjangoModel.cpp \
    QDjangoQuerySet.cpp \
    QDjangoWhere.cpp

# Installation
include(../src.pri)
headers.path = $$PREFIX/include/qdjango/db
QMAKE_PKGCONFIG_INCDIR = $$headers.path
