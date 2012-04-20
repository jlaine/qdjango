set(QT_DONT_USE_QTGUI 1)
set(QT_USE_QTSQL 1)
include(${QT_USE_FILE})

# QDjango database library
set(HEADERS
    QDjango.h
    QDjango_p.h
    QDjangoModel.h
    QDjangoQuerySet.h
    QDjangoQuerySet_p.h
    QDjangoWhere.h)
set(SOURCES
    QDjango.cpp
    QDjangoModel.cpp
    QDjangoQuerySet.cpp
    QDjangoWhere.cpp)

set(MOC_HEADERS
    QDjango_p.h
    QDjangoModel.h)
qt4_wrap_cpp(MOC_SOURCES ${MOC_HEADERS})
add_library(qdjango-db ${QDJANGO_LIBRARY_TYPE} ${SOURCES} ${MOC_SOURCES})
set_target_properties(qdjango-db PROPERTIES SOVERSION 0)
target_link_libraries(qdjango-db ${QT_QTSQL_LIBRARY} ${QT_QTCORE_LIBRARY})

if(NOT QDJANGO_DONT_INSTALL)
    install(FILES ${HEADERS} DESTINATION include/qdjango/db)
    install(TARGETS qdjango-db
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
endif()
