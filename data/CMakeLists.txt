# SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(icons 
    icons/sc-apps-com.rocksandpaper.syndic.svg
    icons/sc-status-com.rocksandpaper.syndic.feed-empty.svg
    icons/sc-actions-com.rocksandpaper.syndic.tray-icon-symbolic.svg
    )
ecm_install_icons(ICONS ${icons} DESTINATION share/icons THEME hicolor)

install(PROGRAMS applications/com.rocksandpaper.syndic.desktop DESTINATION share/applications)
