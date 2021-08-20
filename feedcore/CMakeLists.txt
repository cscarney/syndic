set(feedcore_HEADERS
    feed.h
    article.h
    articleref.h
    storage.h
    future.h
    context.h
    scheduler.h
    allitemsfeed.h
    factory.h
    provisionalfeed.h
    networkaccessmanager.h
    starreditemsfeed.h
    updatablefeed.h
    )
    
set(feedcore_SRCS
    ${feedcore_HEADERS}
    feed.cpp
    article.cpp
    context.cpp
    scheduler.cpp
    allitemsfeed.cpp
    provisionalfeed.cpp
    networkaccessmanager.cpp
    starreditemsfeed.cpp
    updatablefeed.cpp
    )
    
add_library(feedcore STATIC ${feedcore_SRCS})

target_link_libraries(feedcore
    Qt5::Core
    Qt5::Sql
    Qt5::Network
    KF5::Syndication
)
