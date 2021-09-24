# SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

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
    
add_library(sqlite STATIC ${sqlite_SRCS})

target_link_libraries(sqlite
    feedcore
    Qt5::Core
    Qt5::Sql
    KF5::Syndication
)
