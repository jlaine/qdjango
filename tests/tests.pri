include(../qdjango.pri)

QT -= gui
QT += testlib
CONFIG -= app_bundle
CONFIG += testcase

QMAKE_RPATHDIR += $$OUT_PWD/../../../src/db $$OUT_PWD/../../../src/http $$OUT_PWD/../../../src/script
INCLUDEPATH += $$PWD $$QDJANGO_INCLUDEPATH
