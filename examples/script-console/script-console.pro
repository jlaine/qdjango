include(../../qdjango.pri)

QT += script sql

TARGET = qdjango-script-console

INCLUDEPATH += \
    ../../src/db \
    ../../src/script
LIBS += \
    -L../../src/db $$QDJANGO_DB_LIBS \
    -L../../src/script $$QDJANGO_SCRIPT_LIBS
HEADERS += ../../tests/auth-models.h
SOURCES += script-console.cpp ../../tests/auth-models.cpp
