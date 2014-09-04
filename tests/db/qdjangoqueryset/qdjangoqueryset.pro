DEPTH = ../../..
include($${DEPTH}/qdjango.pri)
include($${DEPTH}/tests/tests.pri)
include(../db.pri)

TARGET = tst_qdjangoqueryset
SOURCES += tst_qdjangoqueryset.cpp
