DEPTH = ../../..
include($${DEPTH}/qdjango.pri)
include($${DEPTH}/tests/tests.pri)
include(../db.pri)

TARGET = tst_qdjangometamodel
HEADERS += ../auth-models.h tst_qdjangometamodel.h
SOURCES += ../auth-models.cpp tst_qdjangometamodel.cpp
