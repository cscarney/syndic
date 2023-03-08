# SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Qt5Test REQUIRED)

enable_testing()

add_executable(testStoreAndRetrieveFeed tst_teststoreandretrievefeed.cpp)
add_test(NAME testStoreAndRetrieveFeed COMMAND testStoreAndRetrieveFeed)
target_link_libraries(testStoreAndRetrieveFeed PRIVATE Qt5::Test feedcore sqlite)

add_executable(testUpdateScheduler tst_testupdatescheduler.cpp)
add_test(NAME testUpdateScheduler COMMAND testUpdateScheduler)
target_link_libraries(testUpdateScheduler PRIVATE Qt5::Test feedcore)

add_executable(testContextValuePropagation tst_testcontextvaluepropagation.cpp)
add_test(NAME testContextValuePropagation COMMAND testContextValuePropagation)
target_link_libraries(testContextValuePropagation PRIVATE Qt5::Test feedcore)

add_executable(testProvisionalFeed tst_provisionalFeed.cpp)
add_test(NAME testProvisionalFeed COMMAND testProvisionalFeed)
target_link_libraries(testProvisionalFeed PRIVATE Qt5::Test feedcore)

add_executable(testAllItemsFeed tst_allitemsfeed.cpp)
add_test(NAME testAllItemsFeed COMMAND testAllItemsFeed)
target_link_libraries(testAllItemsFeed PRIVATE Qt5::Test feedcore)
