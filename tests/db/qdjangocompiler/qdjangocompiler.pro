DEPTH = ../../..
include($${DEPTH}/qdjango.pri)
include($${DEPTH}/tests/tests.pri)
include(../db.pri)

TARGET = tst_qdjangocompiler
SOURCES += tst_qdjangocompiler.cpp
