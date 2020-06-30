set(feedstorage_HEADERS
    feedheaders.h
    feeditemheaders.h
    feeditemstatus.h
    storedfeed.h
    storeditem.h
    feedstorage.h
    feeddatabase.h
    sqlitefeedstorage.h
    )
    
set(feedstorage_SRCS
    ${feedstorage_HEADERS}
    feeddatabase.cpp
    sqlitefeedstorage.cpp
    )
    
add_library(feedstorage ${feedstorage_SRCS})

target_link_libraries(feedstorage
    Qt5::Core
    Qt5::Sql
)
