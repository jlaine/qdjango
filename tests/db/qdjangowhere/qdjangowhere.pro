include(../db.pri)

TARGET = tst_qdjangowhere
SOURCES += tst_qdjangowhere.cpp

INCLUDEPATH += $$QDJANGO_INCLUDEPATH

LIBS += -L../../../src/db $$QDJANGO_DB_LIBS
