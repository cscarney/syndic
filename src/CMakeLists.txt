set(feedkeeper_HEADERS
    feedlistmodel.h
    feedmanager.h
    itemmodel.h
    feedupdater.h
    xmlfeedupdater.h
    updatescheduler.h
    dataretriever.h
    loadstatus.h
    )

set(feedkeeper_SRCS
    ${feedkeeper_HEADERS}
    feedlistmodel.cpp
    feedmanager.cpp
    itemmodel.cpp
    feedupdater.cpp
    xmlfeedupdater.cpp
    updatescheduler.cpp
    dataretriever.cpp
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
    KF5::Crash
    KF5::I18n
    KF5::Syndication
    feedstorage
    )


    
