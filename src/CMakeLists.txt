set(QT_DONT_USE_QTGUI 1)
set(QT_USE_QTNETWORK 1)
set(QT_USE_QTSCRIPT 1)
set(QT_USE_QTSQL 1)
include(${QT_USE_FILE})

# Build a shared library by default
if(NOT QDJANGO_LIBRARY_TYPE)
  set(QDJANGO_LIBRARY_TYPE SHARED)
endif()

add_subdirectory(db)
add_subdirectory(http)
add_subdirectory(script)
