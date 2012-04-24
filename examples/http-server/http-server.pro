include(../../qdjango.pri)

QT += network sql

TARGET = qdjango-http-server

INCLUDEPATH += \
    ../../src/db \
    ../../src/http
LIBS += \
    -L../../src/db -lqdjango-db \
    -L../../src/http -lqdjango-http
RESOURCES += http-server.qrc
HEADERS += http-server.h ../../tests/auth-models.h
SOURCES += http-server.cpp ../../tests/auth-models.cpp
