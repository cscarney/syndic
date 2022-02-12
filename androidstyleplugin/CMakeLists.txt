
set(androidstyleplugin_SRCS
    androidstylepluginfactory.h
    androidstylepluginfactory.cpp
    androidstyleplugintheme.h
    androidstyleplugintheme.cpp
    androidstyleplugin.qrc
    )

add_library(androidstyleplugin SHARED ${androidstyleplugin_SRCS})

set_target_properties(androidstyleplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../plugins/kf5/kirigami)
set_target_properties(androidstyleplugin PROPERTIES OUTPUT_NAME plugins_kf5_kirigami_Material)

target_link_libraries(androidstyleplugin Qt5::Core Qt5::Qml Qt5::Quick Qt5::Gui Qt5::AndroidExtras KF5::Kirigami2)
