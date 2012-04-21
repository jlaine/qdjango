# Common definitions
QDJANGO_VERSION=0.2.0

# Determine library type (lib or staticlib)
isEmpty(QDJANGO_LIBRARY_TYPE) {
    QDJANGO_LIBRARY_TYPE = lib
}
contains(QDJANGO_LIBRARY_TYPE,staticlib) {
    DEFINES += QDJANGO_STATIC
} else {
    DEFINES += QDJANGO_SHARED
}
DEFINES += QDJANGO_BUILD
