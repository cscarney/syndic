set(syndic_HEADERS
    feedlistmodel.h
    articlelistmodel.h
    qmlref.h
    qmlarticleref.h
    iconprovider.h
    gumbovisitor.h
    htmlsplitter.h
    contentmodel.h
    notificationcontroller.h
    platformhelper.h
    contentimageitem.h
    application.h
    networkaccessmanagerfactory.h
    )

set(syndic_SRCS
    ${syndic_HEADERS}
    feedlistmodel.cpp
    articlelistmodel.cpp
    iconprovider.cpp
    htmlsplitter.cpp
    contentmodel.cpp
    notificationcontroller.cpp
    gumbovisitor.cpp
    platformhelper.cpp
    contentimageitem.cpp
    networkaccessmanagerfactory.cpp
    application.cpp
    main.cpp
    resources.qrc
    )
    
add_executable(syndic ${syndic_SRCS})

kconfig_add_kcfg_files(syndic settings.kcfgc GENERATE_MOC)

target_link_libraries(syndic
    Qt5::Core
    Qt5::Widgets
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickControls2
    KF5::Syndication
    KF5::ConfigGui
    feedcore
    sqlite
    htmlparser
    )
    
if (KF5Notifications_FOUND)
    target_link_libraries(syndic
        KF5::Notifications
        )
endif()

if(KF5DBusAddons_FOUND)
    target_link_libraries(syndic
        KF5::DBusAddons
        )
endif()

if (ANDROID)
    target_link_libraries(syndic
        KF5::Kirigami2
        OpenSSL::SSL
        Qt5::Svg
        Qt5::AndroidExtras
        )

    kirigami_package_breeze_icons(
        ICONS
        list-add
        settings-configure
        help-about
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
        )
endif()

install(TARGETS syndic DESTINATION bin)

    
