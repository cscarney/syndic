cmake_minimum_required(VERSION 3.5)
project(feedkeeper VERSION 1.0)

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
include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(Qt5 COMPONENTS Core Qml Network Widgets Quick QuickControls2 Sql REQUIRED)

find_package(KF5Declarative CONFIG QUIET)
find_package(KF5Syndication REQUIRED)
find_package(KF5Notifications CONFIG QUIET)
find_package(KF5DBusAddons CONFIG QUIET)

if (ANDROID)
    find_package(KF5Kirigami2 REQUIRED)
    find_package(OpenSSL)
    find_package(Qt5 COMPONENTS Svg AndroidExtras REQUIRED)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror -Wno-error=deprecated-declarations")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

configure_file(cmake-config.h.template ${CMAKE_CURRENT_BINARY_DIR}/cmake-config.h)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} feedcore src)

add_subdirectory(htmlparser)
add_subdirectory(data)
add_subdirectory(feedcore)
add_subdirectory(sqlite)
add_subdirectory(src)
add_subdirectory(test)

