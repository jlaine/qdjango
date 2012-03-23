set(QT_DONT_USE_QTGUI 1)
set(QT_USE_QTNETWORK 1)
set(QT_USE_QTSQL 1)
set(QT_USE_QTTEST 1)
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# test models
set(qdjango-models_SOURCES auth-models.cpp shares-models.cpp)
set(qdjango-models_MOC_HEADERS auth-models.h shares-models.h)

qt4_wrap_cpp(qdjango-models_MOC_SOURCES ${qdjango-models_MOC_HEADERS})
add_library(qdjango-models STATIC ${qdjango-models_SOURCES} ${qdjango-models_MOC_SOURCES})
target_link_libraries(qdjango-models qdjango-db)

# test program
set(qdjango-tests_SOURCES
    fields.cpp
    http.cpp
    main.cpp
    script.cpp
    auth-tests.cpp
    shares-tests.cpp)
set(qdjango-tests_MOC_HEADERS
    fields.h
    http.h
    main.h
    script.h
    auth-tests.h
    shares-tests.h)
qt4_wrap_cpp(qdjango-tests_MOC_SOURCES ${qdjango-tests_MOC_HEADERS})
add_executable(qdjango-tests ${qdjango-tests_SOURCES} ${qdjango-tests_MOC_SOURCES})
target_link_libraries(qdjango-tests qdjango-db qdjango-http qdjango-models qdjango-script ${QT_LIBRARIES})

