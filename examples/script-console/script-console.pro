include(../../qdjango.pri)

QT += script sql

TARGET = qdjango-script-console

INCLUDEPATH += ../../tests/db $$QDJANGO_INCLUDEPATH
LIBS += \
    -L../../src/db $$QDJANGO_DB_LIBS \
    -L../../src/script $$QDJANGO_SCRIPT_LIBS
HEADERS += ../../tests/db/auth-models.h
SOURCES += script-console.cpp ../../tests/db/auth-models.cpp
