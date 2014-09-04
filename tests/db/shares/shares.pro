DEPTH = ../../..
include($${DEPTH}/qdjango.pri)
include($${DEPTH}/tests/tests.pri)
include(../db.pri)

TARGET = tst_shares
SOURCES += tst_shares.cpp
