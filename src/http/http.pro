include(../../qdjango.pri)

TEMPLATE = lib
CONFIG += staticlib
QT += network

TARGET = qdjango-http
VERSION = $$QDJANGO_VERSION
win32 {
    DESTDIR = $$OUT_PWD
}

HEADERS += \
    QDjangoFastCgiServer.h \
    QDjangoFastCgiServer_p.h \
    QDjangoHttpController.h \
    QDjangoHttpRequest.h \
    QDjangoHttpResponse.h \
    QDjangoHttpServer.h \
    QDjangoHttpServer_p.h
SOURCES += \
    QDjangoFastCgiServer.cpp \
    QDjangoHttpController.cpp \
    QDjangoHttpRequest.cpp \
    QDjangoHttpResponse.cpp \
    QDjangoHttpServer.cpp
