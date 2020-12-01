 
file(GLOB icons "icons/*.svg")

ecm_install_icons(ICONS ${icons} DESTINATION share/icons THEME hicolor)
