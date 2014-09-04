include(../../db/db.pri)

QT += script

TARGET = tst_qdjangoscript

HEADERS += ../../db/auth-models.h
SOURCES += ../../db/auth-models.cpp tst_qdjangoscript.cpp

LIBS += -L../../../src/script $$QDJANGO_SCRIPT_LIBS
