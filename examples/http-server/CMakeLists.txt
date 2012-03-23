set(QT_DONT_USE_QTGUI 1)
set(QT_USE_QTNETWORK 1)
set(QT_USE_QTSQL 1)
include(${QT_USE_FILE})

qt4_add_resources(RCC_SOURCES http-server.qrc)
add_executable(qdjango-http-server http-server.cpp ${RCC_SOURCES})
target_link_libraries(qdjango-http-server qdjango-http qdjango-models)
