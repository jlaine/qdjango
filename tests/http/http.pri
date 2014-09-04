include(../../qdjango.pri)
include(../../tests/tests.pri)

QT += network
LIBS += -L$${DEPTH}/src/http $${QDJANGO_HTTP_LIBS}
