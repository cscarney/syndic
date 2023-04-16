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
    updatablefeed.h
    opmlreader.h
    sharedcache.h
    gumbovisitor.h
    feeddiscovery.h
    automation/automationengine.h
    automation/automationrule.h
    readability/readability.h
    readability/readabilityresult.h
    readability/qreadablereadability.h
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
    updatablefeed.cpp
    opmlreader.cpp
    sharedcache.cpp
    gumbovisitor.cpp
    feeddiscovery.cpp
    automation/automationengine.cpp
    automation/automationrule.cpp
    readability/qreadablereadability.cpp
    )
    
add_library(feedcore STATIC ${feedcore_SRCS})
ecm_create_qm_loader(feedcore feedcore)

target_link_libraries(feedcore
    Qt6::Core
    Qt6::Sql
    Qt6::Network
    KF6::Syndication
    QReadable::libqreadable
    htmlparser
)
