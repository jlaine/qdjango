include(../../../qdjango.pri)

QT -= gui
QT += network testlib

TARGET = tst_qdjangohttpserver

SOURCES += tst_qdjangohttpserver.cpp

INCLUDEPATH += $$QDJANGO_INCLUDEPATH

LIBS += -L../../../src/http $$QDJANGO_HTTP_LIBS
