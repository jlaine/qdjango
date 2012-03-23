set(QT_DONT_USE_QTGUI 1)
set(QT_USE_QTSCRIPT 1)
set(QT_USE_QTSQL 1)
include(${QT_USE_FILE})

# QDjango script library
set(HEADERS
    QDjangoScript.h
    QDjangoScript_p.h)
set(SOURCES
    QDjangoScript.cpp)

add_library(qdjango-script ${QDJANGO_LIBRARY_TYPE} ${SOURCES})
set_target_properties(qdjango-script PROPERTIES SOVERSION 0)
target_link_libraries(qdjango-script qdjango-db ${QT_QTSCRIPT_LIBRARY} ${QT_QTCORE_LIBRARY})

if(NOT QDJANGO_DONT_INSTALL)
    install(FILES ${HEADERS} DESTINATION include/qdjango/script)
    install(TARGETS qdjango-script
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
endif()
