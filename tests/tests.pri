include(../qdjango.pri)

QT -= gui
QT += testlib
CONFIG -= app_bundle
CONFIG += testcase

QMAKE_RPATHDIR += ../../../src/db ../../../src/http ../../../src/script
INCLUDEPATH += $$PWD $$QDJANGO_INCLUDEPATH
