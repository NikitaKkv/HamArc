include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    FIND_PACKAGE_ARGS NAMES GTest
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
    hamarc_tests
    test_archiver.cpp
)

enable_testing()

target_link_libraries(
    hamarc_tests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        argparser
        type
        archiver
)

target_compile_definitions(
    hamarc_tests
    PRIVATE
        RESOURCES_DIR=\"${CMAKE_SOURCE_DIR}/tests/resources\"
        HAMARC_EXE_PATH=\"$<TARGET_FILE:hamarc>\"
)

include(GoogleTest)
gtest_discover_tests(hamarc_tests)
