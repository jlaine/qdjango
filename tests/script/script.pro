include(../../qdjango.pri)

QT -= gui
QT += script sql testlib

TARGET = qdjango-script-tests

HEADERS += ../db/auth-models.h script.h
SOURCES += ../db/auth-models.cpp script.cpp

INCLUDEPATH += ../db $$QDJANGO_INCLUDEPATH

LIBS += \
    -L../../src/db $$QDJANGO_DB_LIBS \
    -L../../src/script $$QDJANGO_SCRIPT_LIBS
