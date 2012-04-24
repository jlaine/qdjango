include(../qdjango.pri)

QT += script sql network testlib

TARGET = qdjango-tests

HEADERS += \
    fields.h \
    http.h \
    main.h \
    script.h \
    auth-models.h \
    auth-tests.h \
    shares-models.h \
    shares-tests.h
SOURCES += \
    fields.cpp \
    http.cpp \
    main.cpp \
    script.cpp \
    auth-models.cpp \
    auth-tests.cpp \
    shares-models.cpp \
    shares-tests.cpp

INCLUDEPATH += \
    ../src/db \
    ../src/http \
    ../src/script

LIBS += \
    -L../src/db -lqdjango-db$${QDJANGO_LIBINFIX} \
    -L../src/http -lqdjango-http$${QDJANGO_LIBINFIX} \
    -L../src/script -lqdjango-script$${QDJANGO_LIBINFIX}
