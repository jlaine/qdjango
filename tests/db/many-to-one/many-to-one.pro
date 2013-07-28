include(../db.pri)

TARGET = tst_manytoone
HEADERS += models.h
SOURCES += models.cpp tst_manytoone.cpp

INCLUDEPATH += $$QDJANGO_INCLUDEPATH

LIBS += -L../../../src/db $$QDJANGO_DB_LIBS
