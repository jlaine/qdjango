include(../../../qdjango.pri)

QT -= gui
QT += network testlib

TARGET = tst_qdjangourlresolver

SOURCES += tst_qdjangourlresolver.cpp

INCLUDEPATH += $$QDJANGO_INCLUDEPATH

LIBS += -L../../../src/http $$QDJANGO_HTTP_LIBS
