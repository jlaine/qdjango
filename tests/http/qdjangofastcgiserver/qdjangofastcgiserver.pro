DEPTH = ../../..
include($${DEPTH}/qdjango.pri)
include($${DEPTH}/tests/tests.pri)
include(../http.pri)

TARGET = tst_qdjangofastcgiserver
SOURCES += tst_qdjangofastcgiserver.cpp
