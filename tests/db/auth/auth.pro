DEPTH = ../../..
include($${DEPTH}/qdjango.pri)
include($${DEPTH}/tests/tests.pri)
include(../db.pri)

TARGET = tst_auth
HEADERS += ../auth-models.h
SOURCES += ../auth-models.cpp tst_auth.cpp
