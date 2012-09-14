# Common definitions
QDJANGO_VERSION=0.2.6

# Determine library type (shared or staticlib)
isEmpty(QDJANGO_LIBRARY_TYPE) {
    android {
        QDJANGO_LIBRARY_TYPE = staticlib
    } else {
        QDJANGO_LIBRARY_TYPE = shared
    }
}

# Libraries for apps which use QDjango
QDJANGO_INCLUDEPATH = $$PWD/src/db $$PWD/src/http $$PWD/src/script
QDJANGO_DB_LIBS = -lqdjango-db
QDJANGO_HTTP_LIBS = -lqdjango-http
QDJANGO_SCRIPT_LIBS = -lqdjango-script
contains(QDJANGO_LIBRARY_TYPE,staticlib) {
    DEFINES += QDJANGO_STATIC
} else {
    # Windows needs the major library version
    win32 {
        QDJANGO_DB_LIBS = -lqdjango-db0
        QDJANGO_HTTP_LIBS = -lqdjango-http0
        QDJANGO_SCRIPT_LIBS = -lqdjango-script0
    }
    DEFINES += QDJANGO_SHARED
}

# Installation prefix and library directory
isEmpty(PREFIX) {
    contains(MEEGO_EDITION,harmattan) {
        PREFIX = /usr
    } else:unix {
        PREFIX = /usr/local
    } else {
        PREFIX = $$[QT_INSTALL_PREFIX]
    }
}
isEmpty(LIBDIR) {
    LIBDIR=lib
}
