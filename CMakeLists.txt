# SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.5)
project(syndic VERSION 1.0)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMInstallIcons)
include(ECMPoQmTools)
include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(Qt5 COMPONENTS Core Qml Network Quick QuickControls2 Sql REQUIRED)

find_package(KF5Syndication REQUIRED)
find_package(KF5Config REQUIRED)
find_package(KF5DBusAddons CONFIG QUIET)
find_package(QReadable REQUIRED)

if (ANDROID)
    set(BUILD_TESTING OFF)
    find_package(KF5Kirigami2 REQUIRED)
    find_package(OpenSSL)
    find_package(Qt5 COMPONENTS Svg AndroidExtras REQUIRED)
else()
    find_package(Qt5Widgets CONFIG QUIET)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Werror -Wno-error=deprecated-declarations ${CMAKE_CXX_FLAGS_DEBUG}")
if (BUILD_TESTING)
    set(CMAKE_CXX_FLAGS_DEBUG "--coverage -fprofile-arcs -ftest-coverage ${CMAKE_CXX_FLAGS_DEBUG}")
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

configure_file(cmake-config.h.template ${CMAKE_CURRENT_BINARY_DIR}/cmake-config.h)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 3rdparty feedcore src)

if (ANDROID)
    set(ANDROID_EXTRA_PLUGINS ${CMAKE_BINARY_DIR}/plugins)
    add_subdirectory(androidstyleplugin)
endif()

add_subdirectory(3rdparty/gumbo)
add_subdirectory(data)
add_subdirectory(feedcore)
add_subdirectory(sqlite)
add_subdirectory(src)

ecm_install_po_files_as_qm(po)
install(FILES com.rocksandpaper.syndic.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

if (BUILD_TESTING)
    add_subdirectory(test)
endif()
