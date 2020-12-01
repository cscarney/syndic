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

 find_package(Qt5 COMPONENTS Core Qml Widgets Quick QuickControls2 Sql REQUIRED)
 
 find_package(KF5Declarative REQUIRED)
 find_package(KF5Crash REQUIRED)
 find_package(KF5I18n REQUIRED)
 find_package(KF5Syndication REQUIRED)
  
 set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Werror")
 set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto")
 
 include_directories(feedsource feedstorage src)

 add_subdirectory(data)
 add_subdirectory(feedstorage)
 add_subdirectory(src)

