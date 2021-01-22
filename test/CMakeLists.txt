
find_package(Qt5Test REQUIRED)

enable_testing()

add_executable(testStoreAndRetrieveFeed tst_teststoreandretrievefeed.cpp)
add_test(NAME testStoreAndRetrieveFeed COMMAND testStoreAndRetrieveFeed)
target_link_libraries(testStoreAndRetrieveFeed PRIVATE Qt5::Test feedcore sqlite)

