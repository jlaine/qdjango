include(../tests.pri)

QT += network

LIBS += -L../../../src/http $$QDJANGO_HTTP_LIBS
