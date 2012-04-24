# Common definitions
QDJANGO_VERSION=0.2.0

# Determine library type (shared or staticlib)
isEmpty(QDJANGO_LIBRARY_TYPE) {
    QDJANGO_LIBRARY_TYPE = shared
}
contains(QDJANGO_LIBRARY_TYPE,staticlib) {
    DEFINES += QDJANGO_STATIC
} else {
    DEFINES += QDJANGO_SHARED
    win32:QDJANGO_LIBINFIX=0
}

# Installation prefix and library directory
isEmpty(PREFIX) {
    unix:PREFIX=/usr/local
}
isEmpty(LIBDIR) {
    LIBDIR=lib
}
