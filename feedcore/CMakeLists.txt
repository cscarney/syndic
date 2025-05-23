# SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(feedcore_HEADERS
    feed.h
    article.h
    articleref.h
    storage.h
    future.h
    context.h
    scheduler.h
    aggregatefeed.h
    categoryfeed.h
    factory.h
    provisionalfeed.h
    networkaccessmanager.h
    starreditemsfeed.h
    articlesummary.h
    updatablefeed.h
    opmlreader.h
    sharedcache.h
    gumbovisitor.h
    feeddiscovery.h
    searchresultfeed.h
    articlelinkextractor.h
    highlightsfeed.h
    automation/automationengine.h
    automation/automationrule.h
    readability/readability.h
    readability/readabilityresult.h
    readability/readabilityprefetchrule.h
    )
    
set(feedcore_SRCS
    ${feedcore_HEADERS}
    feed.cpp
    article.cpp
    context.cpp
    scheduler.cpp
    aggregatefeed.cpp
    categoryfeed.cpp
    provisionalfeed.cpp
    networkaccessmanager.cpp
    starreditemsfeed.cpp
    articlesummary.cpp
    updatablefeed.cpp
    opmlreader.cpp
    sharedcache.cpp
    gumbovisitor.cpp
    feeddiscovery.cpp
    searchresultfeed.cpp
    articlelinkextractor.cpp
    highlightsfeed.cpp
    automation/abstractautomationrule.h
    automation/automationengine.cpp
    automation/automationrule.cpp
    readability/readabilityprefetchrule.cpp
    )

if (QReadable_FOUND)
    set(feedcore_SRCS ${feedcore_SRCS}
        readability/qreadablereadability.h
        readability/qreadablereadability.cpp)
else()
    set(feedcore_SRCS ${feedcore_SRCS}
        readability/placeholderreadability.h
        readability/placeholderreadability.cpp)
endif()

add_library(feedcore STATIC ${feedcore_SRCS}
    articlelinkextractor.h
    articlelinkextractor.cpp)
ecm_create_qm_loader(feedcore feedcore)

target_link_libraries(feedcore
    Qt6::Core
    Qt6::Sql
    Qt6::Network
    Qt6::Xml
    KF6::Syndication
    htmlparser
)

if (QReadable_FOUND)
    target_link_libraries(feedcore QReadable::libqreadable)
endif()
