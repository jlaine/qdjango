DEPTH = ../../..
include($${DEPTH}/qdjango.pri)
include($${DEPTH}/tests/tests.pri)
include(../db.pri)

TARGET = tst_qdjango
SOURCES += tst_qdjango.cpp
