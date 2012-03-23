include(qdjango.pri)

TEMPLATE = subdirs

SUBDIRS = src

android {
} else {
    SUBDIR += tests examples
}

CONFIG += ordered
