# SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(syndic_HEADERS
    feedlistmodel.h
    articlelistmodel.h
    qmlref.h
    qmlarticleref.h
    iconprovider.h
    htmlsplitter.h
    contentmodel.h
    notificationcontroller.h
    platformhelper.h
    contentimageitem.h
    application.h
    feedmodel.h
    highlightsmodel.h
    networkaccessmanagerfactory.h
    editablefeedlistmodel.h
    )

set(syndic_SRCS
    ${syndic_HEADERS}
    feedlistmodel.cpp
    articlelistmodel.cpp
    iconprovider.cpp
    htmlsplitter.cpp
    contentmodel.cpp
    notificationcontroller.cpp
    platformhelper.cpp
    contentimageitem.cpp
    networkaccessmanagerfactory.cpp
    application.cpp
    feedmodel.cpp
    highlightsmodel.cpp
    editablefeedlistmodel.cpp
    main.cpp
    resources.qrc
    )
    
add_executable(syndic ${syndic_SRCS})

kconfig_add_kcfg_files(syndic settings.kcfgc GENERATE_MOC)
ecm_create_qm_loader(syndic syndic)

target_link_libraries(syndic
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    KF6::Syndication
    KF6::ConfigGui
    feedcore
    sqlite
    htmlparser
    )

if (Qt6Widgets_FOUND)
    target_link_libraries(syndic
        Qt6::Widgets
        )
endif()

if(KF6DBusAddons_FOUND)
    target_link_libraries(syndic
        KF6::DBusAddons
        )
endif()

if (ANDROID)
    target_link_libraries(syndic
        KF6::Kirigami2
        OpenSSL::SSL
        Qt6::Svg
        Qt6::AndroidExtras
        )

    kirigami_package_breeze_icons(
        ICONS
        list-add
        settings-configure
        help-about
        help-contextual
        feed-subscribe
        content-loading-symbolic
        dialog-error-symbolic
        document-save
        delete
        mail-mark-unread
        starred-symbolic
        non-starred-symbolic
        document-preview
        view-refresh
        view-filter
        document-edit
        globe
        checkmark
        emblem-shared-symbolic
        open-menu-symbolic
        window-close-symbolic
        tools-report-bug
        dialog-cancel
        view-readermode
        )
endif()

install(TARGETS syndic DESTINATION bin)

    
