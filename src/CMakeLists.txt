set(feedkeeper_HEADERS
    feedlistmodel.h
    articlelistmodel.h
    qmlref.h
    qmlarticleref.h
    iconprovider.h
    gumbovisitor.h
    htmlsplitter.h
    contentmodel.h
    newitemnotifier.h
    )

set(feedkeeper_SRCS
    ${feedkeeper_HEADERS}
    feedlistmodel.cpp
    articlelistmodel.cpp
    iconprovider.cpp
    htmlsplitter.cpp
    contentmodel.cpp
    newitemnotifier.cpp
    gumbovisitor.cpp
    main.cpp
    resources.qrc
    )
    
add_executable(feedkeeper ${feedkeeper_SRCS})

target_link_libraries(feedkeeper
    Qt5::Core
    Qt5::Widgets
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickControls2
    KF5::Syndication
    feedcore
    sqlite
    htmlparser
    )
    
if (KF5Notifications_FOUND)
    target_link_libraries(feedkeeper
        KF5::Notifications
        )
endif()

if (KF5Declarative_FOUND)
    target_link_libraries(feedkeeper
        KF5::Declarative
        KF5::QuickAddons
        )
endif()

if (ANDROID)
    target_link_libraries(feedkeeper
        KF5::Kirigami2
        OpenSSL::SSL
        Qt5::Svg
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
        mail-read-symbolic
        mail-read
        view-filter
        document-edit
        globle
        )
endif()

install(TARGETS feedkeeper DESTINATION bin)

    
