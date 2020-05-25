set(feedkeeper_HEADERS
    basicfeedstorage.h
    feedlistmodel.h
    feedmanager.h
    feedsource.h
    feedstorage.h
    itemmodel.h
    ttrssfeedsource.h
    )

set(feedkeeper_SRCS
    ${feedkeeper_HEADERS}
    basicfeedstorage.cpp
    feedlistmodel.cpp
    feedmanager.cpp
    itemmodel.cpp
    main.cpp
    ttrssfeedsource.cpp
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
    KF5::Crash
    KF5::I18n
    KF5::Syndication
    )


    
