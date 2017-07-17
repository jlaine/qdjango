SET(qdjango_db_SRCS
    db/QDjango.cpp
    db/QDjango.h
    db/QDjango_p.h
    db/QDjangoMetaModel.cpp
    db/QDjangoMetaModel.h
    db/QDjangoModel.cpp
    db/QDjangoModel.h
    db/QDjangoQuerySet.cpp
    db/QDjangoQuerySet.h
    db/QDjangoQuerySet_p.h
    db/QDjangoWhere.cpp
    db/QDjangoWhere.h
    db/QDjangoWhere_p.h
)

ADD_LIBRARY(QDjangoDB SHARED ${qdjango_db_SRCS})
TARGET_LINK_LIBRARIES(QDjangoDB Qt5::Core)
TARGET_LINK_LIBRARIES(QDjangoDB Qt5::Sql)
