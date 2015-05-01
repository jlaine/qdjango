include(../../qdjango.pri)

QT -= gui
QT += network

DEFINES += QDJANGO_HTTP_BUILD

TARGET = qdjango-http
win32 {
    DESTDIR = $$OUT_PWD
}

HEADERS += \
    QDjangoFastCgiServer.h \
    QDjangoFastCgiServer_p.h \
    QDjangoHttp_p.h \
    QDjangoHttpController.h \
    QDjangoHttpRequest.h \
    QDjangoHttpResponse.h \
    QDjangoHttpServer.h \
    QDjangoHttpServer_p.h \
    QDjangoUrlResolver.h
SOURCES += \
    QDjangoFastCgiServer.cpp \
    QDjangoHttpController.cpp \
    QDjangoHttpRequest.cpp \
    QDjangoHttpResponse.cpp \
    QDjangoHttpServer.cpp \
    QDjangoUrlResolver.cpp

# Installation
include(../src.pri)
headers.path = $$PREFIX/include/qdjango/http
QMAKE_PKGCONFIG_INCDIR = $$headers.path
