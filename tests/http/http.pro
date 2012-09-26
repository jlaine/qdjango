include(../../qdjango.pri)

QT -= gui
QT += network testlib

TARGET = qdjango-http-tests

HEADERS += http.h
SOURCES += http.cpp

INCLUDEPATH += $$QDJANGO_INCLUDEPATH

LIBS += -L../../src/http $$QDJANGO_HTTP_LIBS
