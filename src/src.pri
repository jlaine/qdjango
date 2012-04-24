include(../qdjango.pri)

TEMPLATE = lib
CONFIG += $$QDJANGO_LIBRARY_TYPE
DEFINES += QDJANGO_BUILD
VERSION = $$QDJANGO_VERSION

QMAKE_CLEAN += lib$${TARGET}.prl

# Installation
headers.files = $$HEADERS
target.path = $$PREFIX/$$LIBDIR
INSTALLS += headers target

# pkg-config support
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_LIBDIR = $$target.path
