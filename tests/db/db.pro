include(../../qdjango.pri)

QT -= gui
QT += sql testlib

TARGET = qdjango-db-tests

HEADERS += \
    main.h \
    auth-models.h \
    auth-tests.h \
    util.h
SOURCES += \
    main.cpp \
    auth-models.cpp \
    auth-tests.cpp \
    util.cpp

INCLUDEPATH += $$QDJANGO_INCLUDEPATH

LIBS += -L../../src/db $$QDJANGO_DB_LIBS
