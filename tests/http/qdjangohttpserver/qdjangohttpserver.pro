DEPTH = ../../..
include($${DEPTH}/qdjango.pri)
include($${DEPTH}/tests/tests.pri)
include(../http.pri)

TARGET = tst_qdjangohttpserver
SOURCES += tst_qdjangohttpserver.cpp
