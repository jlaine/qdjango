include(../qdjango.pri)

QT -= gui
QT += script sql network testlib

TARGET = qdjango-tests

HEADERS += \
    fields.h \
    foreignkey.h \
    main.h \
    auth-models.h \
    auth-tests.h \
    shares-models.h \
    shares-tests.h
SOURCES += \
    fields.cpp \
    foreignkey.cpp \
    main.cpp \
    auth-models.cpp \
    auth-tests.cpp \
    shares-models.cpp \
    shares-tests.cpp

INCLUDEPATH += $$QDJANGO_INCLUDEPATH

LIBS += \
    -L../src/db $$QDJANGO_DB_LIBS \
    -L../src/http $$QDJANGO_HTTP_LIBS \
    -L../src/script $$QDJANGO_SCRIPT_LIBS
