# SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(ttrss_HEADERS
    ttrssclient.h
    storageimpl.h
    feedimpl.h
    articleimpl.h
    )
    
set(ttrss_SRCS
    ${ttrss_HEADERS}
    storageimpl.cpp
    ttrssclient.cpp
    feedimpl.cpp
    articleimpl.cpp
    )
    
add_library(ttrss STATIC ${ttrss_SRCS})

target_link_libraries(ttrss
    feedcore
    Qt5::Core
    Qt5::Network
)
