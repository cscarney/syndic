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
    gumbovisitor.cpp
    htmlsplitter.cpp
    contentmodel.cpp
    newitemnotifier.cpp
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
    KF5::Declarative
    KF5::QuickAddons
    KF5::Syndication
    KF5::Notifications
    feedcore
    sqlite
    gumbo
    )

install(TARGETS feedkeeper DESTINATION bin)

    
