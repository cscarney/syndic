set(feedcore_HEADERS
    feed.h
    feedref.h
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
    uniquefactory.h
    preview/articleimpl.h
    provisionalfeed.h
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
    )
    
add_library(feedcore ${feedcore_SRCS})

target_link_libraries(feedcore
    Qt5::Core
    Qt5::Sql
    Qt5::Network
    KF5::Syndication
)
