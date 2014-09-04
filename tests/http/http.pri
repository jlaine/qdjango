include(../../qdjango.pri)

QT -= gui
QT += network testlib

INCLUDEPATH += $$QDJANGO_INCLUDEPATH
LIBS += -L../../../src/http $$QDJANGO_HTTP_LIBS
