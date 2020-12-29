set(sqlite_HEADERS
    feedquery.h
    itemquery.h
    articleimpl.h
    feeddatabase.h
    feedimpl.h
    storageimpl.h
    )
    
set(sqlite_SRCS
    ${sqlite_HEADERS}
    articleimpl.cpp
    feeddatabase.cpp
    feedimpl.cpp
    storageimpl.cpp
    )
    
add_library(sqlite ${sqlite_SRCS})

target_link_libraries(sqlite
    feedcore
    Qt5::Core
    Qt5::Sql
    KF5::Syndication
)
