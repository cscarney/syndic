 
set(icons 
    icons/sc-apps-feedkeeper.svg
    icons/sc-status-feedkeeper-feed-empty.svg)
message(${icons})
ecm_install_icons(ICONS ${icons} DESTINATION share/icons THEME hicolor)

install(PROGRAMS applications/feedkeeper.desktop DESTINATION share/applications)
