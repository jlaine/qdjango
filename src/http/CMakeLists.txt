set(QT_DONT_USE_QTGUI 1)
set(QT_USE_QTNETWORK 1)
include(${QT_USE_FILE})

# QDjango http library
set(HEADERS
    QDjangoFastCgiServer.h
    QDjangoFastCgiServer_p.h
    QDjangoHttpController.h
    QDjangoHttpRequest.h
    QDjangoHttpResponse.h
    QDjangoHttpServer.h
    QDjangoHttpServer_p.h)
set(SOURCES
    QDjangoFastCgiServer.cpp
    QDjangoHttpController.cpp
    QDjangoHttpRequest.cpp
    QDjangoHttpResponse.cpp
    QDjangoHttpServer.cpp)

set(MOC_HEADERS
    QDjangoFastCgiServer.h
    QDjangoFastCgiServer_p.h
    QDjangoHttpResponse.h
    QDjangoHttpServer.h
    QDjangoHttpServer_p.h)
qt4_wrap_cpp(MOC_SOURCES ${MOC_HEADERS})
add_library(qdjango-http ${QDJANGO_LIBRARY_TYPE} ${SOURCES} ${MOC_SOURCES})
set_target_properties(qdjango-http PROPERTIES SOVERSION 0)
target_link_libraries(qdjango-http ${QT_QTNETWORK_LIBRARY} ${QT_QTCORE_LIBRARY})

if(NOT QDJANGO_DONT_INSTALL)
    install(FILES ${HEADERS} DESTINATION include/qdjango/http)
    install(TARGETS qdjango-http
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
endif()
