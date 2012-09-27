include(../../qdjango.pri)

QT += network sql

TARGET = qdjango-http-server

INCLUDEPATH += ../../tests/db $$QDJANGO_INCLUDEPATH
LIBS += \
    -L../../src/db $$QDJANGO_DB_LIBS \
    -L../../src/http $$QDJANGO_HTTP_LIBS
RESOURCES += http-server.qrc
HEADERS += http-server.h ../../tests/db/auth-models.h
SOURCES += http-server.cpp ../../tests/db/auth-models.cpp
