set(feedkeeper_HEADERS
    feedlistmodel.h
    itemmodel.h
    allitemmodel.h
    feeditemmodel.h
    managedlistmodel.h
    feedrefwrapper.h
    )

set(feedkeeper_SRCS
    ${feedkeeper_HEADERS}
    feedlistmodel.cpp
    itemmodel.cpp
    allitemmodel.cpp
    feeditemmodel.cpp
    managedlistmodel.cpp
    feedrefwrapper.cpp
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
    feedcore
    )

install(TARGETS feedkeeper DESTINATION bin)

    
