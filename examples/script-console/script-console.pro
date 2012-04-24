include(../../qdjango.pri)

QT += script sql

TARGET = qdjango-script-console

INCLUDEPATH += \
    ../../src/db \
    ../../src/script
LIBS += \
    -L../../src/db -lqdjango-db$${QDJANGO_LIBINFIX} \
    -L../../src/script -lqdjango-script$${QDJANGO_LIBINFIX}
HEADERS += ../../tests/auth-models.h
SOURCES += script-console.cpp ../../tests/auth-models.cpp
