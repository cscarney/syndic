# SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(testplugin`_HEADERS
    testplugin.h
    )
    
set(testplugin_SRCS
    ${testplugin_HEADERS}
    testplugin.cpp
    )
    
add_library(testplugin SHARED ${testplugin_SRCS})

target_link_libraries(testplugin
    feedcore
    Qt6::Core
    KF6::Syndication
)
