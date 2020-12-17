set(feedcore_HEADERS
    feed.h
    feedref.h
    article.h
    articleref.h
    feedstorage.h
    feeddatabase.h
    sqlitefeedstorage.h
    sqlitefeed.h
    sqlitearticle.h
    feedstorageoperation.h
    context.h
    feedupdater.h
    xmlfeedupdater.h
    updatescheduler.h
    dataretriever.h
    enums.h
    allitemsfeed.h
    )
    
set(feedcore_SRCS
    ${feedcore_HEADERS}
    feed.cpp
    article.cpp
    feeddatabase.cpp
    sqlitefeedstorage.cpp
    sqlitefeed.cpp
    sqlitearticle.cpp
    context.cpp
    feedupdater.cpp
    xmlfeedupdater.cpp
    updatescheduler.cpp
    dataretriever.cpp
    allitemsfeed.cpp
    )
    
add_library(feedcore ${feedcore_SRCS})

target_link_libraries(feedcore
    Qt5::Core
    Qt5::Sql
    Qt5::Network
    KF5::Syndication
)
