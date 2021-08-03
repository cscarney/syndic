set(feedcore_HEADERS
    feed.h
    article.h
    articleref.h
    storage.h
    future.h
    context.h
    updater.h
    xmlupdater.h
    scheduler.h
    dataretriever.h
    enums.h
    allitemsfeed.h
    factory.h
    preview/articleimpl.h
    provisionalfeed.h
    cachednetworkaccessmanager.h
    starreditemsfeed.h
    )
    
set(feedcore_SRCS
    ${feedcore_HEADERS}
    feed.cpp
    article.cpp
    context.cpp
    updater.cpp
    xmlupdater.cpp
    scheduler.cpp
    dataretriever.cpp
    allitemsfeed.cpp
    preview/articleimpl.cpp
    provisionalfeed.cpp
    cachednetworkaccessmanager.cpp
    starreditemsfeed.cpp
    )
    
add_library(feedcore STATIC ${feedcore_SRCS})

target_link_libraries(feedcore
    Qt5::Core
    Qt5::Sql
    Qt5::Network
    KF5::Syndication
)
