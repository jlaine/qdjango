include(../../../qdjango.pri)

QT -= gui
QT += network testlib

TARGET = tst_qdjangohttprequest

SOURCES += tst_qdjangohttprequest.cpp

INCLUDEPATH += $$QDJANGO_INCLUDEPATH

LIBS += -L../../../src/http $$QDJANGO_HTTP_LIBS
