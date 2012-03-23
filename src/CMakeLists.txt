set(QT_DONT_USE_QTGUI 1)
set(QT_USE_QTNETWORK 1)
set(QT_USE_QTSCRIPT 1)
set(QT_USE_QTSQL 1)
include(${QT_USE_FILE})

# Needed for plugin support
if(UNIX)
  if(NOT APPLE)
    add_definitions(-fPIC)
  endif(NOT APPLE)
endif(UNIX)

# Until binary stability is reached, build static libraries
if(NOT QDJANGO_LIBRARY_TYPE)
  set(QDJANGO_LIBRARY_TYPE SHARED)
endif()

add_subdirectory(db)
add_subdirectory(http)
add_subdirectory(script)
