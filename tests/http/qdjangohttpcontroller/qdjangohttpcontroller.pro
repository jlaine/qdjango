include(../../../qdjango.pri)

QT -= gui
QT += network testlib

TARGET = tst_qdjangohttpcontroller

SOURCES += tst_qdjangohttpcontroller.cpp
RESOURCES += tst_qdjangohttpcontroller.qrc

INCLUDEPATH += $$QDJANGO_INCLUDEPATH

LIBS += -L../../../src/http $$QDJANGO_HTTP_LIBS
