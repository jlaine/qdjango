include(../qdjango.pri)

QT -= gui
QT += testlib
CONFIG -= app_bundle
CONFIG += testcase

unix:!macx:QMAKE_RPATHDIR += \
    $$OUT_PWD/../../../src/db \
    $$OUT_PWD/../../../src/http \
    $$OUT_PWD/../../../src/script

macx {
    # this doesn't current work on osx, but may be in the future
    QMAKE_RPATHDIR += \
        @loader_path/../../../src/db \
        @loader_path/../../../src/http \
        @loader_path/../../../src/script

    QMAKE_LFLAGS += \
        -Wl,-rpath,@loader_path/../../../src/db \
        -Wl,-rpath,@loader_path/../../../src/http \
        -Wl,-rpath,@loader_path/../../../src/script
}

INCLUDEPATH += $$PWD $$QDJANGO_INCLUDEPATH
