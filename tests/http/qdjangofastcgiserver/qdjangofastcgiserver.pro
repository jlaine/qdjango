include(../../../qdjango.pri)

QT -= gui
QT += network testlib

TARGET = tst_qdjangofastcgiserver

SOURCES += tst_qdjangofastcgiserver.cpp

INCLUDEPATH += $$QDJANGO_INCLUDEPATH

LIBS += -L../../../src/http $$QDJANGO_HTTP_LIBS
