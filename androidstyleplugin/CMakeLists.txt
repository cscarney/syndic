# SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(androidstyleplugin_SRCS
    androidstylepluginfactory.h
    androidstylepluginfactory.cpp
    androidstyleplugintheme.h
    androidstyleplugintheme.cpp
    androidstylepluginiconloader.h
    androidstylepluginiconloader.cpp
    androidstyleplugin.qrc
    )

add_library(androidstyleplugin SHARED ${androidstyleplugin_SRCS})

set_target_properties(androidstyleplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../plugins/kf6/kirigami)
set_target_properties(androidstyleplugin PROPERTIES OUTPUT_NAME plugins_kf6_kirigami_MaterialTweaks)

target_link_libraries(androidstyleplugin Qt6::Core Qt6::Qml Qt6::Quick Qt6::Gui Qt6::AndroidExtras KF6::Kirigami2)
