include(../../qdjango.pri)

QT += network sql

TARGET = qdjango-http-server

INCLUDEPATH += \
    ../../src/db \
    ../../src/http
LIBS += \
    -L../../src/db -lqdjango-db$${QDJANGO_LIBINFIX} \
    -L../../src/http -lqdjango-http$${QDJANGO_LIBINFIX}
RESOURCES += http-server.qrc
HEADERS += http-server.h ../../tests/auth-models.h
SOURCES += http-server.cpp ../../tests/auth-models.cpp
