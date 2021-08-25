 
set(icons 
    icons/sc-apps-syndic.svg
    icons/sc-status-syndic-feed-empty.svg)
message(${icons})
ecm_install_icons(ICONS ${icons} DESTINATION share/icons THEME hicolor)

install(PROGRAMS applications/com.rocksandpaper.syndic.desktop DESTINATION share/applications)
