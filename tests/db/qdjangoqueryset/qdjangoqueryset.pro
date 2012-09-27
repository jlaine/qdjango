include(../db.pri)

TARGET = tst_qdjangoqueryset
SOURCES += tst_qdjangoqueryset.cpp

INCLUDEPATH += $$QDJANGO_INCLUDEPATH

LIBS += -L../../../src/db $$QDJANGO_DB_LIBS
