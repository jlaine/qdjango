DEPTH = ../../..
include($${DEPTH}/qdjango.pri)
include($${DEPTH}/tests/tests.pri)
include(../http.pri)

TARGET = tst_qdjangohttpcontroller
SOURCES += tst_qdjangohttpcontroller.cpp
RESOURCES += tst_qdjangohttpcontroller.qrc
