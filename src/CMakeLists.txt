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
    
qt_add_executable(syndic ${syndic_SRCS})

kconfig_add_kcfg_files(syndic settings.kcfgc GENERATE_MOC)
ecm_create_qm_loader(syndic syndic)

target_link_libraries(syndic PRIVATE
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
    target_link_libraries(syndic PRIVATE
        Qt6::Widgets
        )
endif()

if(KF6DBusAddons_FOUND)
    target_link_libraries(syndic PRIVATE
        KF6::DBusAddons
        )
endif()

if(KF6BreezeIcons_FOUND)
    target_link_libraries(syndic
        KF6::BreezeIcons
        )
endif()

if (APPLE)
    set(SYNDIC_MACOS_MIN_VERSION "${CMAKE_OSX_DEPLOYMENT_TARGET}")
    if (NOT SYNDIC_MACOS_MIN_VERSION)
        # Qt 6.10 from Homebrew requires macOS 12 or newer.
        set(SYNDIC_MACOS_MIN_VERSION "12.0")
    endif()

    set(SYNDIC_ICNS "${CMAKE_SOURCE_DIR}/macos/syndic.icns")
    target_sources(syndic PRIVATE ${SYNDIC_ICNS})
    set_source_files_properties(${SYNDIC_ICNS}
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    set_target_properties(syndic PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/macos/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "Syndic"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.rocksandpaper.syndic"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_ICON_FILE "syndic.icns"
        MACOSX_BUNDLE_COPYRIGHT "Copyright © Connor Carney. GPL-3.0-or-later."
        )
endif()

if (ANDROID)
    # On Android the app is built as a MODULE library that androiddeployqt
    # expects to be named lib<target>_<abi>.so, but KDECMakeSettings clears
    # CMAKE_SHARED_MODULE_PREFIX; restore the prefix or the APK step can't
    # find the binary.
    if (NOT ANDROID_VERSION_CODE)
        set(ANDROID_VERSION_CODE 1)
    endif()

    # The package source dir is assembled in the build tree (see the
    # android-package-source target in the top-level CMakeLists) so that
    # generated files like the translation catalogs can be added to the
    # package alongside the static files from android/. Prime it at configure
    # time; the target refreshes it on every build.
    file(GLOB android_package_files ${CMAKE_SOURCE_DIR}/android/*)
    file(COPY ${android_package_files} DESTINATION ${CMAKE_BINARY_DIR}/android-package)

    set_target_properties(syndic PROPERTIES
        PREFIX "lib"
        QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_BINARY_DIR}/android-package
        QT_ANDROID_VERSION_NAME ${PROJECT_VERSION}
        QT_ANDROID_VERSION_CODE ${ANDROID_VERSION_CODE}
        )

    # androiddeployqt only scans Qt's own qml dir for modules; KDE frameworks
    # install theirs under lib/qml, so the Kirigami QML module won't be
    # bundled without this.
    foreach(prefix ${CMAKE_PREFIX_PATH})
        if (EXISTS ${prefix}/lib/qml)
            set_property(TARGET syndic APPEND PROPERTY QT_QML_IMPORT_PATH ${prefix}/lib/qml)
        endif()
    endforeach()

    target_link_libraries(syndic PRIVATE
        KF6::Kirigami
        OpenSSL::SSL
        Qt6::Svg
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

    
