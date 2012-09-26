include(../../qdjango.pri)

QT -= gui
QT += sql testlib

TARGET = qdjango-db-tests

HEADERS += \
    fields.h \
    foreignkey.h \
    main.h \
    auth-models.h \
    auth-tests.h \
    shares-models.h \
    shares-tests.h \
    util.h
SOURCES += \
    fields.cpp \
    foreignkey.cpp \
    main.cpp \
    auth-models.cpp \
    auth-tests.cpp \
    shares-models.cpp \
    shares-tests.cpp \
    util.cpp

INCLUDEPATH += $$QDJANGO_INCLUDEPATH

LIBS += -L../../src/db $$QDJANGO_DB_LIBS
