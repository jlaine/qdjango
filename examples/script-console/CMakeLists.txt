set(QT_DONT_USE_QTGUI 1)
set(QT_USE_QTSCRIPT 1)
set(QT_USE_QTSQL 1)
include(${QT_USE_FILE})

add_executable(qdjango-script-console script-console.cpp)
target_link_libraries(qdjango-script-console qdjango-models qdjango-script)
