include(../../../qdjango.pri)

QT -= gui
QT += script sql testlib

TARGET = tst_qdjangoscript

HEADERS += ../../db/auth-models.h ../../db/util.h
SOURCES += ../../db/auth-models.cpp ../../db/util.cpp tst_qdjangoscript.cpp

INCLUDEPATH += ../../db $$QDJANGO_INCLUDEPATH

LIBS += \
    -L../../../src/db $$QDJANGO_DB_LIBS \
    -L../../../src/script $$QDJANGO_SCRIPT_LIBS
