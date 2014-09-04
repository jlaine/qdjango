include(../qdjango.pri)

QT -= gui
QT += testlib
CONFIG -= app_bundle
CONFIG += testcase

INCLUDEPATH += $$PWD $$QDJANGO_INCLUDEPATH
