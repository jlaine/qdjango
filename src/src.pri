TEMPLATE = lib
CONFIG += $$QDJANGO_LIBRARY_TYPE
VERSION = $$QDJANGO_VERSION

# Installation
headers.files = $$HEADERS
target.path = $$PREFIX/$$LIBDIR
INSTALLS += headers target

# pkg-config support
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_LIBDIR = $$target.path
equals(QDJANGO_LIBRARY_TYPE,staticlib) {
    QMAKE_PKGCONFIG_CFLAGS = -DQDJANGO_STATIC
} else {
    QMAKE_PKGCONFIG_CFLAGS = -DQDJANGO_SHARED
}
unix:QMAKE_CLEAN += -r pkgconfig lib$${TARGET}.prl

# profiling support
equals(QDJANGO_PROFILE,true) {
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_LIBS += -lgcov
    QMAKE_CLEAN += *.gcda *.gcov *.gcno
}
