set(feedcore_HEADERS
    feedheaders.h
    feeditemheaders.h
    feeditemstatus.h
    feed.h
    feedref.h
    storeditem.h
    feedstorage.h
    feeddatabase.h
    sqlitefeedstorage.h
    sqlitefeed.h
    feedstorageoperation.h
    context.h
    feedupdater.h
    xmlfeedupdater.h
    updatescheduler.h
    dataretriever.h
    enums.h
    )
    
set(feedcore_SRCS
    ${feedcore_HEADERS}
    feed.cpp
    feeddatabase.cpp
    sqlitefeedstorage.cpp
    sqlitefeed.cpp
    context.cpp
    feedupdater.cpp
    xmlfeedupdater.cpp
    updatescheduler.cpp
    dataretriever.cpp

    )
    
add_library(feedcore ${feedcore_SRCS})

target_link_libraries(feedcore
    Qt5::Core
    Qt5::Sql
    Qt5::Network
    KF5::Syndication
)
