set(feedsource_HEADERS
    feedsource.h
    ttrssfeedsource.h
    )
    
set(feedsource_SRCS
    ${feedsource_HEADERS}
    ttrssfeedsource.cpp
    )
    
add_library(feedsource ${feedsource_SRCS})

target_link_libraries(feedsource
    Qt5::Core
    Qt5::Network
    KF5::Syndication
)
