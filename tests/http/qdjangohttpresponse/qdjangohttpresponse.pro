DEPTH = ../../..
include($${DEPTH}/qdjango.pri)
include($${DEPTH}/tests/tests.pri)
include(../http.pri)

TARGET = tst_qdjangohttpresponse
SOURCES += tst_qdjangohttpresponse.cpp
