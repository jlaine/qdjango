DEPTH = ../../..
include($${DEPTH}/qdjango.pri)
include($${DEPTH}/tests/tests.pri)

QT += script sql

TARGET = tst_qdjangoscript
HEADERS += ../../db/auth-models.h ../../db/util.h
SOURCES += ../../db/auth-models.cpp ../../db/util.cpp tst_qdjangoscript.cpp

INCLUDEPATH += ../../db $$QDJANGO_INCLUDEPATH
LIBS += \
    -L$${DEPTH}/src/db $${QDJANGO_DB_LIBS} \
    -L$${DEPTH}/src/script $${QDJANGO_SCRIPT_LIBS}
