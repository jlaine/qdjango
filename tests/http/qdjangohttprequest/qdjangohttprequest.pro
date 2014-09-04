DEPTH = ../../..
include($${DEPTH}/qdjango.pri)
include($${DEPTH}/tests/tests.pri)
include(../http.pri)

TARGET = tst_qdjangohttprequest
SOURCES += tst_qdjangohttprequest.cpp
